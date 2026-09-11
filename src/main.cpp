#include "isobus/hardware_integration/can_hardware_interface.hpp"
#include "isobus/hardware_integration/twai_plugin.hpp"

#include "isobus/isobus/can_network_manager.hpp"
#include "isobus/isobus/can_partnered_control_function.hpp"
#include "isobus/isobus/isobus_virtual_terminal_client.hpp"
#include "isobus/isobus/isobus_task_controller_server.hpp"
#include "isobus/isobus/isobus_task_controller_client.hpp"
#include "isobus/isobus/isobus_device_descriptor_object_pool.hpp"
#include "isobus/isobus/isobus_standard_data_description_indices.hpp"
#include "isobus/isobus/can_stack_logger.hpp"
#include "isobus/isobus/can_message.hpp"
#include "isobus/isobus/isobus_device_descriptor_object_pool.hpp"

#include "isobus/isobus/isobus_virtual_terminal_client.hpp"
#include "isobus/isobus/isobus_virtual_terminal_client_update_helper.hpp"

#include "isobus/isobus/can_parameter_group_number_request_protocol.hpp"

#include "isobus/utility/iop_file_interface.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "../VT_object_pool/Zirkon12.iop.h"
#include "lemken_zirkon12_ddop.hpp"

#include <cstdint>
#include <cstdio>
#include <limits>
#include <memory>
#include <vector>
#include <atomic>
#include <memory>
#include <csignal>
#include <iostream>

//#include "micro_ros_transport.hpp"

extern "C" const uint8_t object_pool_start[] asm("_binary_Zirkon12_iop_start");
extern "C" const uint8_t object_pool_end[]   asm("_binary_Zirkon12_iop_end");

static std::shared_ptr<isobus::InternalControlFunction> g_myECU;
static std::shared_ptr<isobus::PartneredControlFunction> g_partnerVT;
static std::shared_ptr<isobus::PartneredControlFunction> g_partnerTC;

// Initialize struct to adress relay id and gpio pin number
struct Relay { uint16_t id; gpio_num_t gpio; };
static Relay relays[] = {
	{6001, GPIO_NUM_8},
};
static std::vector<double> transmission_ratio = {0.23, 0.33};
std::uint16_t SELECTED_GEAR = 0; // 0 translates to Gear 1 and 1 translates to Gear 2, used for easy indexing of transmission ratio

class SimpleLogger : public isobus::CANStackLogger
{
public:
	void sink_CAN_stack_log(CANStackLogger::LoggingLevel level, const std::string &text) override
	{
		printf("[AgIsoStack %d] %s\n", static_cast<int>(level), text.c_str());
	}
};

static SimpleLogger g_logger;
static std::shared_ptr<isobus::VirtualTerminalClient> g_vt;
static std::shared_ptr<isobus::VirtualTerminalClientUpdateHelper> g_vtUpdateHelper;

// --- Task controller / DDOP state ---
static std::shared_ptr<isobus::TaskControllerClient> g_tcClient;
static LemkenZirkon12DDOP g_ddopHandler;
static std::shared_ptr<isobus::DeviceDescriptorObjectPool> g_ddop = std::make_shared<isobus::DeviceDescriptorObjectPool>(3);
static bool g_prevPtoEngaged = false;

// helper constants for the rear pto speed PGN
static constexpr std::uint16_t PTO_SPEED_NOT_AVAILABLE = 0xFFFF;
static constexpr std::uint16_t PTO_SPEED_ERROR_INDICATOR = 0xFE00;
static constexpr TickType_t PTO_REQUEST_INTERVAL = pdMS_TO_TICKS(1000);
static std::uint32_t g_lastPtoRpm = std::numeric_limits<std::uint32_t>::max();

// Below what RPM the PTO is considered "not turning" for work-state purposes.
// Tune this if the shaft idles/coasts a bit above zero when disengaged.
static constexpr std::uint32_t PTO_ENGAGED_RPM_THRESHOLD = 50;

static void init_relays()
{
	for (auto &r : relays)
	{
		gpio_config_t cfg = {};
		cfg.intr_type = GPIO_INTR_DISABLE;
		cfg.mode = GPIO_MODE_OUTPUT;
		cfg.pin_bit_mask = (1ULL << r.gpio);
		gpio_config(&cfg);
		gpio_set_level(r.gpio, 0);
		printf("Relay on GPIO %d configured\n", r.gpio);
	}
}

static void set_relay(uint16_t id, bool on)
{
	for (auto &r : relays)
	{
		if (r.id == id)
		{
			gpio_set_level(r.gpio, on ? 1 : 0);
			printf("Relay %u -> %s\n", id, on ? "ON" : "OFF");
			return;
		}
	}
}

/*
* @brief Round number to the nearest multiple of the integer multiple.
* @param[in] number The number to be rounded.
* @param[in] multiple The multiple to round to.
* @return The rounded number.
*/
std::uint32_t roundToNearestMultiple(std::uint32_t number, std::uint32_t multiple) {
    return ((number + (multiple / 2)) / multiple) * multiple;
}

/*
* @brief Updates the numeric value for the tine RPM on the VT.
* @param[in] rpm The new tine RPM value to update on the VT.
*/
void update_tine_numeric_value(std::uint32_t rpm)
{
	if (nullptr != g_vtUpdateHelper)
	{
		g_vtUpdateHelper->set_numeric_value(NV_Tine_RPM, rpm);
	}
}

/*
* @brief Updates the numeric value for the PTO RPM on the VT.
* @param[in] rpm The new PTO RPM value to update on the VT.
*/
void update_pto_numeric_value(std::uint32_t rpm)
{
	if ((nullptr != g_vtUpdateHelper) && (rpm != g_lastPtoRpm))
	{
		g_lastPtoRpm = rpm;
		g_vtUpdateHelper->set_numeric_value(NV_PTO_RPM, rpm);
	}
}

/*
* @brief Updates the DDOP's PTO-engaged state and, on a rising/falling edge, tells the
*        TC client to push the new Actual Work State to the task controller immediately
*        rather than waiting for its next on-change poll.
* @param[in] rpm The current rear PTO output shaft speed in RPM.
*/
void update_pto_engagement_state(std::uint32_t rpm)
{
	bool engaged = (rpm > PTO_ENGAGED_RPM_THRESHOLD);
	g_ddopHandler.set_pto_engaged(engaged);

	if ((engaged != g_prevPtoEngaged) && (nullptr != g_tcClient))
	{
		g_tcClient->on_value_changed_trigger(static_cast<std::uint16_t>(ImplementDDOPElementNumbers::DeviceElement),
		                                      static_cast<std::uint16_t>(isobus::DataDescriptionIndex::ActualWorkState));
		g_prevPtoEngaged = engaged;
	}
}


/*
* @brief Function that gets called when the wheel-based machine speed PGN and the PTO speed PGN are received. It calculates the tine RPM per meter and updates the VT with the new value.
*/
void calc_tineRPM_per_meter()
{
	float machine_speed_kmh = 0.0f;
	machine_speed_kmh = g_vtUpdateHelper->get_numeric_value(NV_speed) * 0.1;

	// Convert the machine speed from km/h to m/s
	float machine_speed_meter_per_second = machine_speed_kmh / 3.6f;

	// Convert the RPM to revolutions per second
	std::uint32_t tine_rev_per_minute = g_vtUpdateHelper->get_numeric_value(NV_Tine_RPM);
	float tine_rev_per_second = static_cast<float>(tine_rev_per_minute) / 60.0f;

	// Avoid dividing by zero when the machine is stopped.
	if (machine_speed_meter_per_second <= 0.0f || tine_rev_per_second <= 0)
	{
		//micro_ros_publish_tine_rpm_per_meter(0.0f);
		g_vtUpdateHelper->set_numeric_value(NV_tine_rpm_per_m_21006, 0);
		return;
	}

	// Calculate the tine RPM per meter only after validating the denominator.
	uint32_t tine_rpm_per_meter_scaled = static_cast<uint32_t>((tine_rev_per_second / machine_speed_meter_per_second) * 10.0f);
	//micro_ros_publish_tine_rpm_per_meter(static_cast<float>(tine_rpm_per_meter_scaled) * 0.1f);
	g_vtUpdateHelper->set_numeric_value(NV_tine_rpm_per_m_21006, tine_rpm_per_meter_scaled);
}

/*
* @brief Callback function to handle the rear PTO output shaft speed PGN.
* @param[in] message The CAN message containing the rear PTO output shaft speed PGN.
*/
void handle_rear_pto_output_shaft_message(const isobus::CANMessage &message, void *)
{
	if (message.get_data_length() < 2)
	{
		return;
	}

	std::uint16_t raw_speed = static_cast<std::uint16_t>(message.get_data()[0] | (message.get_data()[1] << 8));
	if (raw_speed == PTO_SPEED_NOT_AVAILABLE || raw_speed == PTO_SPEED_ERROR_INDICATOR)
	{
		return;
	}

	// SPN 1883 is scaled as 0.125 rpm/bit, so divide by 8 and round to the nearest whole rpm for the VT.
	std::uint32_t rpm = (static_cast<std::uint32_t>(raw_speed) + 4U) / 8U;
	rpm = roundToNearestMultiple(rpm, 10); // Round to the nearest 10 RPM for display purposes

	// Update the PTO and tine RPM numeric values on the VT
	update_pto_numeric_value(rpm);

	// Send rpm via Micro-ROS2 publisher
	//micro_ros_publish_pto_rpm(static_cast<float>(rpm));

	// Calculate the tine RPM based on the selected gear and update the VT
	std::uint32_t tine_rpm = static_cast<std::uint32_t>(static_cast<double>(rpm) * transmission_ratio[SELECTED_GEAR]);
	update_tine_numeric_value(tine_rpm);

	// Send tine_rpm via Micro-ROS2 publisher
	//micro_ros_publish_tine_rpm(static_cast<float>(tine_rpm));

	// Trigger the calculation of tine RPM per meter based on the updated PTO and ground-based machine speed values
	calc_tineRPM_per_meter();

	// Drive the DDOP's Actual Work State from PTO engagement, and notify the TC on transitions
	update_pto_engagement_state(rpm);
}

/*
* @brief Callback function to handle the ground-based machine speed PGN (65097).
* @param[in] message The CAN message containing the ground-based machine speed PGN.
* @details SPN 1859 (Ground-based machine speed) is in bytes 1-2 with 0.1 m/s per bit scale.
*/
void handle_wheel_based_speed_message(const isobus::CANMessage &message, void *)
{
	// SPN 1859: Ground-based machine speed (bytes 1-2, unsigned 16-bit, scale 0.1 m/s/bit, offset 0)
	std::uint16_t raw_speed = static_cast<std::uint16_t>(message.get_data()[0] | (message.get_data()[1] << 8));

	// Convert raw value to m/s using scale factor 0.1 m/s per bit
	float speed_mps = static_cast<float>(raw_speed) * 0.01f;

	// Convert speed from m/s to km/h
	float speed_kmh = speed_mps * 3.6f;

	// Store as uint32 with 1 decimal place (multiply by 10)
	std::uint32_t speed_scaled = static_cast<std::uint32_t>(speed_kmh + 0.5f);

	// Send speed_scaled as wheelbased speed via Micro-ROS2 publisher
	//micro_ros_publish_wheelbased_speed_kmh(static_cast<float>(speed_kmh) * 0.1f);

	std::uint32_t use_gnss = g_vtUpdateHelper->get_numeric_value(NV_use_gnss_bool);
	if (use_gnss < 1)
	{
		g_vtUpdateHelper->set_numeric_value(NV_speed, speed_scaled);
	}
}
/*
* @brief Callback function to handle GNSS-based vehicle speed messages (PGN 0xFEE8).
* @param[in] message The CAN message containing the GNSS speed PGN.
* @details NavigationBasedVehicleSpeed: bits 16-31 (bytes 2-3), unsigned 16-bit,
*          little-endian, scale 0.00390625 km/h/bit, offset 0, range [0, 255.996] km/h.
*/
void handle_gnss_speed_message(const isobus::CANMessage &message, void *)
{
	if (message.get_data_length() < 4)
	{
		return;
	}

	std::uint16_t raw_speed = static_cast<std::uint16_t>(message.get_data()[2] | (message.get_data()[3] << 8));

	// Signal is already in km/h - apply the DBC scale factor directly, no unit conversion needed.
	float speed_kmh = static_cast<float>(raw_speed) * 0.00390625f;

	// Store scaled by 10 to preserve one decimal place for the VT's 0.1 presentation object
	// (e.g. 12.3 km/h -> 123, displayed as 12.3 after the object pool's 0.1 scaling).
	std::uint32_t speed_scaled = static_cast<std::uint32_t>(speed_kmh *10 + 0.5f); // +0.5 for round-to-nearest

	// Send speed_scaled as GNSS-based speed via Micro-ROS2 publisher
	//micro_ros_publish_gnss_speed_kmh(static_cast<float>(speed_kmh));

	std::uint32_t use_gnss = g_vtUpdateHelper->get_numeric_value(NV_use_gnss_bool);
	if (use_gnss > 0)
	{
		g_vtUpdateHelper->set_numeric_value(NV_speed, speed_scaled);
	}
}

/*
* @brief Callback function to handle button events on the VT.
* @param[in] event The virtual terminal key event.
*/
void handle_button_event(const isobus::VirtualTerminalClient::VTKeyEvent &event)
{
	switch (event.objectID)
	{
		case btn_light:
		{
			if (event.keyEvent == isobus::VirtualTerminalClient::KeyActivationCode::ButtonPressedOrLatched)
			{
				set_relay(btn_light, true);
				break;
			}

			if (event.keyEvent == isobus::VirtualTerminalClient::KeyActivationCode::ButtonUnlatchedOrReleased)
			{
				set_relay(btn_light, false);
				break;
			}
			break;
		}

		case btn_gear_change:
		{
			if (event.keyEvent == isobus::VirtualTerminalClient::KeyActivationCode::ButtonPressedOrLatched)
			{
				SELECTED_GEAR = 1;
				g_vtUpdateHelper->set_numeric_value(NV_btn_gear, 2);
				g_vtUpdateHelper->set_numeric_value(NV_sel_gear, 2);
				break;
			}
			if (event.keyEvent == isobus::VirtualTerminalClient::KeyActivationCode::ButtonUnlatchedOrReleased)
			{
				SELECTED_GEAR = 0;
				g_vtUpdateHelper->set_numeric_value(NV_btn_gear, 1);
				g_vtUpdateHelper->set_numeric_value(NV_sel_gear, 1);
				break;
			}
			break;
		}
		default:
			break;
	}
}


static void init_task(void *)
{
	g_tcClient = std::make_shared<isobus::TaskControllerClient>(g_partnerTC, g_myECU, g_partnerVT);

	if (!LemkenZirkon12DDOP::create_ddop(g_ddop, g_myECU->get_NAME()))
	{
		printf("Failed to build Zirkon 12 DDOP\n");
	}
	else
	{
		std::vector<std::uint8_t> serializedDDOP;
		bool serialized = g_ddop->generate_binary_object_pool(serializedDDOP);
		printf("DDOP built OK: objects=%u, version=%u, serialized=%s, bytes=%u\n",
		       g_ddop->size(),
		       g_ddop->get_task_controller_compatibility_level(),
		       serialized ? "yes" : "no",
		       static_cast<unsigned>(serializedDDOP.size()));

		g_tcClient->configure(g_ddop,
		                       /* maxNumberBoomsSupported = */ 1,
		                       /* maxNumberSectionsSupported = */ 1,
		                       /* maxNumberChannelsSupportedForPositionBasedControl = */ 0,
		                       /* reportToTCSupportsDocumentation = */ false,
		                       /* reportToTCSupportsTCGEOWithoutPositionBasedControl = */ false,
		                       /* reportToTCSupportsTCGEOWithPositionBasedControl = */ false,
		                       /* reportToTCSupportsPeerControlAssignment = */ false,
		                       /* reportToTCSupportsImplementSectionControl = */ true);

		g_tcClient->add_default_process_data_requested_callback(LemkenZirkon12DDOP::default_process_data_request_callback, &g_ddopHandler);
		g_tcClient->add_request_value_callback(LemkenZirkon12DDOP::request_value_command_callback, &g_ddopHandler);
		g_tcClient->add_value_command_callback(LemkenZirkon12DDOP::value_command_callback, &g_ddopHandler);
		g_tcClient->initialize(true);

		printf("TC client initialized\n");
	}

	printf("TC initialization task finished\n");
	vTaskDelete(nullptr);
}

extern "C" void app_main()
{
	// Set up the logger for the stack
	isobus::CANStackLogger::set_can_stack_logger_sink(&g_logger);
	isobus::CANStackLogger::set_log_level(isobus::CANStackLogger::LoggingLevel::Debug);
	setvbuf(stdout, nullptr, _IONBF, 0);
	printf("app_main start\n");

	// Set up the CAN driver and start the stack
	twai_general_config_t gcfg = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_4, GPIO_NUM_5, TWAI_MODE_NORMAL);
	twai_timing_config_t tcfg = TWAI_TIMING_CONFIG_250KBITS();
	twai_filter_config_t fcfg = TWAI_FILTER_CONFIG_ACCEPT_ALL();
	auto driver = std::make_shared<isobus::TWAIPlugin>(&gcfg, &tcfg, &fcfg);
	isobus::CANHardwareInterface::set_number_of_can_channels(1);
	isobus::CANHardwareInterface::assign_can_channel_frame_handler(0, driver);
	isobus::CANHardwareInterface::set_periodic_update_interval(10);
	if ((!isobus::CANHardwareInterface::start()) || (!driver->get_is_valid()))
	{
		printf("CAN start failed\n");
		while (true) vTaskDelay(pdMS_TO_TICKS(1000));
	}

	vTaskDelay(pdMS_TO_TICKS(250));

	//------------------------------------------------------------------
	// Create the internal control function
	isobus::NAME myNAME(0);
	myNAME.set_arbitrary_address_capable(true);
	// Agriculture & Forestry
	myNAME.set_industry_group(2);

	// Secondary Tillage
	myNAME.set_device_class(3);

	// Secondary Tillage Machine Control
	myNAME.set_function_code(132);

	// Set the identity number (arbitrary choice for this example)
	myNAME.set_identity_number(12);

	// Set the manufacturer code to Lemken (196)
	myNAME.set_manufacturer_code(196);

	// Create our InternalControlFunction
	auto myECU = isobus::CANNetworkManager::CANNetwork.create_internal_control_function(myNAME, 0); // desiredName = myName, CANPort = 0
	g_myECU = myECU;
	
	//------------------------------------------------------------------
	// Partnered control function for the tractor ECU
	isobus::NAME tractorNAME(0);
	tractorNAME.set_arbitrary_address_capable(true);
	tractorNAME.set_industry_group(2);  // Agricultural and Forestry Equipment
	tractorNAME.set_device_class(0);    // Agricultural Tractor
	tractorNAME.set_function_code(static_cast<std::uint8_t>(isobus::NAME::Function::Engine));
	tractorNAME.set_identity_number(1);
	auto tractor_ECU = isobus::CANNetworkManager::CANNetwork.create_partnered_control_function(0, { isobus::NAMEFilter(isobus::NAME::NAMEParameters::FunctionCode, static_cast<std::uint8_t>(isobus::NAME::Function::Engine))});

	// Create a PartneredControlFunction for the Virtual Terminal
	std::vector<isobus::NAMEFilter> vtFilters = { isobus::NAMEFilter(isobus::NAME::NAMEParameters::FunctionCode, static_cast<std::uint8_t>(isobus::NAME::Function::VirtualTerminal)) };
	auto partnerVT = isobus::CANNetworkManager::CANNetwork.create_partnered_control_function(0, vtFilters); //CANPort = 0, filters = vtFilters
	g_partnerVT = partnerVT;

	// Create a PartneredControlFunction for the Task Controller
	const isobus::NAMEFilter filterTaskController(isobus::NAME::NAMEParameters::FunctionCode, static_cast<std::uint8_t>(isobus::NAME::Function::TaskController));
	const isobus::NAMEFilter filterTaskControllerInstance(isobus::NAME::NAMEParameters::FunctionInstance, 0);
	const isobus::NAMEFilter filterTaskControllerIndustryGroup(isobus::NAME::NAMEParameters::IndustryGroup, static_cast<std::uint8_t>(isobus::NAME::IndustryGroup::AgriculturalAndForestryEquipment));
	const isobus::NAMEFilter filterTaskControllerDeviceClass(isobus::NAME::NAMEParameters::DeviceClass, static_cast<std::uint8_t>(isobus::NAME::DeviceClass::NonSpecific));
	const std::vector<isobus::NAMEFilter> tcNameFilters = { filterTaskController,
															filterTaskControllerInstance,
															filterTaskControllerIndustryGroup,
															filterTaskControllerDeviceClass };
	auto partnerTC = isobus::CANNetworkManager::CANNetwork.create_partnered_control_function(0, tcNameFilters); //CANPort = 0, filters = tcNameFilters
	g_partnerTC = partnerTC;

	// Create the Virtual Terminal Client and Update Helper
	auto vt = std::make_shared<isobus::VirtualTerminalClient>(partnerVT, myECU);
	auto vt_helper = std::make_shared<isobus::VirtualTerminalClientUpdateHelper>(vt);
	g_vt = vt;
	g_vtUpdateHelper = vt_helper;

	// Initialize the VT with the object pool
	const uint8_t *poolStart = object_pool_start;
	const uint8_t *poolEnd   = object_pool_end;
	std::size_t poolSize = static_cast<std::size_t>(poolEnd - poolStart);
	vt->set_object_pool(0, poolStart, static_cast<std::uint32_t>(poolSize), "VTPOOL");
	vt->initialize(true);

	// Used to keep the Output Numbers always at the value of the corresponding Numbver Variables, so that the VT always shows the correct value of the Output Numbers
	if (nullptr != g_vtUpdateHelper)
	{
		g_vtUpdateHelper->add_tracked_numeric_value(NV_PTO_RPM, 0);
		g_vtUpdateHelper->add_tracked_numeric_value(NV_Tine_RPM, 0);
		g_vtUpdateHelper->add_tracked_numeric_value(NV_btn_gear, 1);
		g_vtUpdateHelper->add_tracked_numeric_value(NV_sel_gear, 1);
		//g_vtUpdateHelper->add_tracked_numeric_value(NV_wheelbased_speed, 0);
		//g_vtUpdateHelper->add_tracked_numeric_value(NV_gnss_speed, 0);
		g_vtUpdateHelper->add_tracked_numeric_value(NV_tine_rpm_per_m_21006, 0);
		g_vtUpdateHelper->add_tracked_numeric_value(NV_speed, 0);
		g_vtUpdateHelper->add_tracked_numeric_value(NV_use_gnss_bool, 1);

		g_vtUpdateHelper->initialize();
	}

	// Initialize the Relays
	init_relays();

	// Set the callback for the softkey events
	vt->get_vt_button_event_dispatcher().add_listener(handle_button_event);

	//------------------------------------------------------------------
	// Set up task Controller Client
	// DDOP creation and TC startup use significant stack space. Keep this work
	// off the app_main task and give it enough room for the TC worker startup.
	TaskHandle_t ddopTask = nullptr;
	BaseType_t taskResult = xTaskCreatePinnedToCore(init_task, "ddop_init", 16384, nullptr, 4, &ddopTask, 1);
	if (taskResult != pdPASS)
	{
		printf("Failed to create DDOP initialization task\n");
	}

	//------------------------------------------------------------------
	// Request the rear PTO output shaft speed every 500 ms
	int pto_pgn = 0xFE43;
	isobus::ParameterGroupNumberRequestProtocol::request_repetition_rate(pto_pgn, 100, myECU, tractor_ECU);
	// Register a message callback for the rear PTO output shaft speed PGN
	isobus::CANNetworkManager::CANNetwork.add_any_control_function_parameter_group_number_callback(pto_pgn, handle_rear_pto_output_shaft_message, nullptr);

	//------------------------------------------------------------------
	// Request the wheel-based machine speed every 500 ms
	int wheel_based_speed_pgn = 0xFE48;
	isobus::ParameterGroupNumberRequestProtocol::request_repetition_rate(wheel_based_speed_pgn, 500, myECU, nullptr);
	// Register a message callback for the wheel-based machine speed PGN
	isobus::CANNetworkManager::CANNetwork.add_any_control_function_parameter_group_number_callback(wheel_based_speed_pgn, handle_wheel_based_speed_message, nullptr);

	//------------------------------------------------------------------
	// Request the GNSS speed every 500 ms
	int gnss_speed_pgn = 0xFEE8;
	isobus::ParameterGroupNumberRequestProtocol::request_repetition_rate(gnss_speed_pgn, 500, myECU, nullptr);
	// Register a message callback for the GNSS speed PGN
	isobus::CANNetworkManager::CANNetwork.add_any_control_function_parameter_group_number_callback(gnss_speed_pgn, handle_gnss_speed_message, nullptr);

	// Start the Micro-ROS2 transport
	//micro_ros_start();

	bool prevConnected = false;
	bool prevTCConnected = false;
	bool prevTaskActive = false;
	TickType_t lastStatusPrint = xTaskGetTickCount();
	while (true)
	{
		bool connected = vt->get_is_connected();
		if (connected && !prevConnected)
		{
			vt->set_object_pool(0, poolStart, static_cast<std::uint32_t>(poolSize), "VTPOOL");
			printf("VT connected, pool assigned\n");
		}
		prevConnected = connected;

		if (nullptr != g_tcClient)
		{
			bool tcConnected = g_tcClient->get_is_connected();
			bool taskActive = g_tcClient->get_is_task_active();
			if ((tcConnected != prevTCConnected) || (taskActive != prevTaskActive))
			{
				printf("TC status: connected=%s, task_active=%s\n",
				       tcConnected ? "yes" : "no",
				       taskActive ? "yes" : "no");
				prevTCConnected = tcConnected;
				prevTaskActive = taskActive;
			}
		}

		if ((xTaskGetTickCount() - lastStatusPrint) >= pdMS_TO_TICKS(5000))
		{
			printf("Status: VT connected=%s, TC connected=%s, task active=%s, DDOP objects=%u\n",
			       connected ? "yes" : "no",
			       (nullptr != g_tcClient) && g_tcClient->get_is_connected() ? "yes" : "no",
			       (nullptr != g_tcClient) && g_tcClient->get_is_task_active() ? "yes" : "no",
			       g_ddop->size());
			lastStatusPrint = xTaskGetTickCount();
		}

		vTaskDelay(pdMS_TO_TICKS(50));
	}
	isobus::CANHardwareInterface::stop();
}