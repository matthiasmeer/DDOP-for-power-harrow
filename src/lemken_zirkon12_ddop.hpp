#ifndef LEMKEN_ZIRKON12_DDOP_HPP
#define LEMKEN_ZIRKON12_DDOP_HPP

#include "isobus/isobus/isobus_device_descriptor_object_pool.hpp"
#include "isobus/isobus/isobus_task_controller_client.hpp"
#include "isobus/isobus/can_NAME.hpp"

#include <cstdint>
#include <memory>

// Fixed implement geometry
static constexpr std::int32_t IMPLEMENT_WIDTH_MM = 3000; // 3 m power harrow, single section
static constexpr std::uint8_t NUMBER_OF_SECTIONS = 1;

// Object IDs used within the DDOP. Values just need to be unique within the pool;
// using an incrementing enum (like the AgIsoStack seeder example) keeps them readable.
enum class ImplementDDOPObjectIDs : std::uint16_t
{
	Device = 0,
	MainDeviceElement = 1,
	DeviceActualWorkState,
	RequestDefaultProcessData,

	Connector,
	ConnectorXOffset,
	ConnectorYOffset,
	ConnectorType,

	MainImplement,
	ImplementXOffset,
	ImplementYOffset,
	ImplementZOffset,
	ActualWorkingWidth,
	SetpointWorkState,

	Section1,
	Section1XOffset,
	Section1YOffset,
	Section1Width,
	Section1ActualWorkState,
	Section1SetpointWorkState,

	ShortWidthPresentation,
	LongWidthPresentation
};

// Element numbers as referenced by the TC client callbacks (must match add_device_element order)
enum class ImplementDDOPElementNumbers : std::uint16_t
{
	DeviceElement = 0,
	ConnectorElement = 1,
	ImplementElement = 2,
	SectionElement = 3
};

class LemkenZirkon12DDOP
{
public:
	static bool create_ddop(std::shared_ptr<isobus::DeviceDescriptorObjectPool> poolToPopulate, isobus::NAME clientName);

	// Called by the application whenever the tractor's rear PTO engagement state changes.
	// The harrow is only "working" while the PTO is turning.
	void set_pto_engaged(bool engaged);
	bool get_pto_engaged() const;

	bool get_setpoint_work_state() const;

	// TC client callbacks
	static bool default_process_data_request_callback(std::uint16_t elementNumber,
	                                                    std::uint16_t DDI,
	                                                    isobus::TaskControllerClient::DefaultProcessDataSettings &returnedSettings,
	                                                    void *parentPointer);

	static bool request_value_command_callback(std::uint16_t elementNumber,
	                                            std::uint16_t DDI,
	                                            std::int32_t &value,
	                                            void *parentPointer);

	static bool value_command_callback(std::uint16_t elementNumber,
	                                    std::uint16_t DDI,
	                                    std::int32_t processVariableValue,
	                                    void *parentPointer);

private:
	bool ptoEngaged = false;
	bool setpointWorkState = false;
};

#endif // LEMKEN_ZIRKON12_DDOP_HPP
