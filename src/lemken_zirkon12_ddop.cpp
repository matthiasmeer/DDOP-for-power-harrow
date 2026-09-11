#include "lemken_zirkon12_ddop.hpp"
#include "isobus/isobus/can_constants.hpp"
#include "isobus/isobus/isobus_standard_data_description_indices.hpp"
#include "isobus/utility/system_timing.hpp"
#include "isobus/utility/to_string.hpp"

#include <array>

static std::string VERSION_STRING = "LZ12v11";

void LemkenZirkon12DDOP::set_pto_engaged(bool engaged)
{
	ptoEngaged = engaged;
	setpointWorkState = engaged; // No auto/manual switch logic needed for a rigid harrow - PTO is the only "on/off"
}

bool LemkenZirkon12DDOP::get_pto_engaged() const
{
	return ptoEngaged;
}

bool LemkenZirkon12DDOP::get_setpoint_work_state() const
{
	return setpointWorkState;
}

bool LemkenZirkon12DDOP::create_ddop(std::shared_ptr<isobus::DeviceDescriptorObjectPool> poolToPopulate, isobus::NAME clientName)
{
	bool retVal = true;
	std::uint16_t elementCounter = 0;

	poolToPopulate->clear();

	// English, decimal point, 24 hour time, ddmmyyyy, metric units
	constexpr std::array<std::uint8_t, 7> localizationData = { 'e', 'n', 0b01010000, 0x00, 0b01010101, 0b01010101, 0xFF };

	// Device element
	retVal &= poolToPopulate->add_device(
		"Lemken Zirkon 12",
		"1.0.1", "ZIRKON12-3M",
		VERSION_STRING,
		localizationData,
		std::vector<std::uint8_t>(),
		clientName.get_full_name()
	);
	retVal &= poolToPopulate->add_device_element("Zirkon 12", elementCounter, 0,
													isobus::task_controller_object::DeviceElementObject::Type::Device,
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::MainDeviceElement));

	retVal &= poolToPopulate->add_device_process_data("Actual Work State",
	                                                   static_cast<std::uint16_t>(isobus::DataDescriptionIndex::ActualWorkState),
	                                                   isobus::NULL_OBJECT_ID,
	                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::PropertiesBit::MemberOfDefaultSet),
	                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::AvailableTriggerMethods::OnChange),
	                                                   static_cast<std::uint16_t>(ImplementDDOPObjectIDs::DeviceActualWorkState));

	retVal &= poolToPopulate->add_device_process_data("Request Default PD",
	                                                   static_cast<std::uint16_t>(isobus::DataDescriptionIndex::RequestDefaultProcessData),
	                                                   isobus::NULL_OBJECT_ID,
	                                                   0,
	                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::AvailableTriggerMethods::Total),
	                                                   static_cast<std::uint16_t>(ImplementDDOPObjectIDs::RequestDefaultProcessData));
	elementCounter++;

	// Connector element (3-point hitch)
	retVal &= poolToPopulate->add_device_element("Connector", elementCounter, static_cast<std::uint16_t>(ImplementDDOPObjectIDs::MainDeviceElement),
													isobus::task_controller_object::DeviceElementObject::Type::Connector,
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Connector));
	retVal &= poolToPopulate->add_device_process_data("Connector X",
	                                                   static_cast<std::uint16_t>(isobus::DataDescriptionIndex::DeviceElementOffsetX),
	                                                   static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ShortWidthPresentation),
	                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::PropertiesBit::Settable),
	                                                   0,
	                                                   static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ConnectorXOffset));
	retVal &= poolToPopulate->add_device_process_data("Connector Y",
	                                                   static_cast<std::uint16_t>(isobus::DataDescriptionIndex::DeviceElementOffsetY),
	                                                   static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ShortWidthPresentation),
	                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::PropertiesBit::Settable),
	                                                   0,
	                                                   static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ConnectorYOffset));
	// Connector type 3 = ISO 730 three-point-hitch mounted implement.
	retVal &= poolToPopulate->add_device_property("Type", 3, static_cast<std::uint16_t>(isobus::DataDescriptionIndex::ConnectorType),
													isobus::NULL_OBJECT_ID,
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ConnectorType));
	elementCounter++;

	// Implement (function) element - the harrow frame itself
	retVal &= poolToPopulate->add_device_element("Zirkon 12 Frame", elementCounter,
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::MainDeviceElement),
													isobus::task_controller_object::DeviceElementObject::Type::Function,
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::MainImplement));
	retVal &= poolToPopulate->add_device_property("Offset X", 0,
													static_cast<std::uint16_t>(isobus::DataDescriptionIndex::DeviceElementOffsetX),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ShortWidthPresentation),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ImplementXOffset));
	retVal &= poolToPopulate->add_device_property("Offset Y", 0,
													static_cast<std::uint16_t>(isobus::DataDescriptionIndex::DeviceElementOffsetY),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ShortWidthPresentation),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ImplementYOffset));
	retVal &= poolToPopulate->add_device_property("Offset Z", 0,
													static_cast<std::uint16_t>(isobus::DataDescriptionIndex::DeviceElementOffsetZ),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ShortWidthPresentation),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ImplementZOffset));

	retVal &= poolToPopulate->add_device_process_data("Actual Working Width",
	                                                   static_cast<std::uint16_t>(isobus::DataDescriptionIndex::ActualWorkingWidth),
	                                                   static_cast<std::uint16_t>(ImplementDDOPObjectIDs::LongWidthPresentation),
	                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::PropertiesBit::MemberOfDefaultSet),
	                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::AvailableTriggerMethods::OnChange),
	                                                   static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ActualWorkingWidth));

	retVal &= poolToPopulate->add_device_process_data("Setpoint Work State",
	                                                   static_cast<std::uint16_t>(isobus::DataDescriptionIndex::SetpointWorkState),
	                                                   isobus::NULL_OBJECT_ID,
	                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::PropertiesBit::MemberOfDefaultSet) | static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::PropertiesBit::Settable),
	                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::AvailableTriggerMethods::OnChange),
	                                                   static_cast<std::uint16_t>(ImplementDDOPObjectIDs::SetpointWorkState));
	elementCounter++;

	// Section (1 section spanning the full 3 m width)
	retVal &= poolToPopulate->add_device_element("Section 1", elementCounter,
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::MainImplement),
													isobus::task_controller_object::DeviceElementObject::Type::Section,
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1));
	retVal &= poolToPopulate->add_device_property("Offset X", 0, static_cast<std::uint16_t>(isobus::DataDescriptionIndex::DeviceElementOffsetX),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::LongWidthPresentation),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1XOffset));
	retVal &= poolToPopulate->add_device_property("Offset Y", 0, static_cast<std::uint16_t>(isobus::DataDescriptionIndex::DeviceElementOffsetY),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::LongWidthPresentation),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1YOffset));
	retVal &= poolToPopulate->add_device_process_data("Section Actual Work State",
                                                   static_cast<std::uint16_t>(isobus::DataDescriptionIndex::ActualWorkState),
                                                   isobus::NULL_OBJECT_ID,
                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::PropertiesBit::MemberOfDefaultSet),
                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::AvailableTriggerMethods::OnChange),
                                                   static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1ActualWorkState));
	retVal &= poolToPopulate->add_device_process_data("Section Setpoint Work State",
                                                   static_cast<std::uint16_t>(isobus::DataDescriptionIndex::SetpointWorkState),
                                                   isobus::NULL_OBJECT_ID,
                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::PropertiesBit::MemberOfDefaultSet) | static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::PropertiesBit::Settable),
                                                   static_cast<std::uint8_t>(isobus::task_controller_object::DeviceProcessDataObject::AvailableTriggerMethods::OnChange),
                                                   static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1SetpointWorkState));
	retVal &= poolToPopulate->add_device_property("Width", IMPLEMENT_WIDTH_MM,
													static_cast<std::uint16_t>(isobus::DataDescriptionIndex::ActualWorkingWidth),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::LongWidthPresentation),
													static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1Width));



	if (retVal)
	{
		auto section = std::static_pointer_cast<isobus::task_controller_object::DeviceElementObject>(
		poolToPopulate->get_object_by_id(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1)));
		section->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1XOffset));
		section->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1YOffset));
		section->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1Width));
		section->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1ActualWorkState));
		section->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Section1SetpointWorkState));
	}
	elementCounter++;

	// Presentations
	retVal &= poolToPopulate->add_device_value_presentation("mm", 0, 1.0f, 0, static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ShortWidthPresentation));
	retVal &= poolToPopulate->add_device_value_presentation("m", 0, 0.001f, 0, static_cast<std::uint16_t>(ImplementDDOPObjectIDs::LongWidthPresentation));

	if (retVal)
	{
		auto device = std::static_pointer_cast<isobus::task_controller_object::DeviceElementObject>(poolToPopulate->get_object_by_id(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::MainDeviceElement)));
		auto connector = std::static_pointer_cast<isobus::task_controller_object::DeviceElementObject>(poolToPopulate->get_object_by_id(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::Connector)));
		auto implement = std::static_pointer_cast<isobus::task_controller_object::DeviceElementObject>(poolToPopulate->get_object_by_id(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::MainImplement)));

		device->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::DeviceActualWorkState));
		device->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::RequestDefaultProcessData));

		connector->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ConnectorXOffset));
		connector->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ConnectorYOffset));
		connector->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ConnectorType));

		implement->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ImplementXOffset));
		implement->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ImplementYOffset));
		implement->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ImplementZOffset));
		implement->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::ActualWorkingWidth));
		implement->add_reference_to_child_object(static_cast<std::uint16_t>(ImplementDDOPObjectIDs::SetpointWorkState));
	}

	return retVal;
}

bool LemkenZirkon12DDOP::default_process_data_request_callback(std::uint16_t elementNumber,
                                                                 std::uint16_t DDI,
                                                                 isobus::TaskControllerClient::DefaultProcessDataSettings &returnedSettings,
                                                                 void *parentPointer)
{
	// printf("TC default-PD request: element=%u DDI=%u\n", elementNumber, DDI);
	bool retVal = false;

	if (nullptr != parentPointer)
	{
		if (static_cast<std::uint16_t>(ImplementDDOPElementNumbers::DeviceElement) == elementNumber &&
		    static_cast<std::uint16_t>(isobus::DataDescriptionIndex::ActualWorkState) == DDI)
		{
			returnedSettings.enableChangeThresholdTrigger = true;
			returnedSettings.changeThreshold = 1;
			retVal = true;
		}
		else if (static_cast<std::uint16_t>(ImplementDDOPElementNumbers::ImplementElement) == elementNumber &&
		         (static_cast<std::uint16_t>(isobus::DataDescriptionIndex::ActualWorkingWidth) == DDI ||
		          static_cast<std::uint16_t>(isobus::DataDescriptionIndex::SetpointWorkState) == DDI))
		{
			returnedSettings.enableChangeThresholdTrigger = true;
			returnedSettings.changeThreshold = 1;
			retVal = true;
		}
		else if (static_cast<std::uint16_t>(ImplementDDOPElementNumbers::SectionElement) == elementNumber &&
				(static_cast<std::uint16_t>(isobus::DataDescriptionIndex::ActualWorkState) == DDI ||
				static_cast<std::uint16_t>(isobus::DataDescriptionIndex::SetpointWorkState) == DDI))
		{
			returnedSettings.enableChangeThresholdTrigger = true;
			returnedSettings.changeThreshold = 1;
			retVal = true;
		}
	}

	return retVal;
}

bool LemkenZirkon12DDOP::request_value_command_callback(std::uint16_t elementNumber,
                                                          std::uint16_t DDI,
                                                          std::int32_t &value,
                                                          void *parentPointer)
{
	if (nullptr != parentPointer)
	{
		auto sim = reinterpret_cast<LemkenZirkon12DDOP *>(parentPointer);

		switch (DDI)
		{
			case static_cast<std::uint16_t>(isobus::DataDescriptionIndex::ActualWorkState):
			{
				// The harrow is only actually working while the PTO is engaged.
				value = sim->get_pto_engaged() ? 1 : 0;
			}
			break;

			case static_cast<std::uint16_t>(isobus::DataDescriptionIndex::SetpointWorkState):
			{
				value = sim->get_setpoint_work_state() ? 1 : 0;
			}
			break;

			case static_cast<std::uint16_t>(isobus::DataDescriptionIndex::ActualWorkingWidth):
			{
				value = IMPLEMENT_WIDTH_MM;
			}
			break;

			case static_cast<std::uint16_t>(isobus::DataDescriptionIndex::DeviceElementOffsetX):
			case static_cast<std::uint16_t>(isobus::DataDescriptionIndex::DeviceElementOffsetY):
			case static_cast<std::uint16_t>(isobus::DataDescriptionIndex::DeviceElementOffsetZ):
			case static_cast<std::uint16_t>(isobus::DataDescriptionIndex::RequestDefaultProcessData):
			{
				value = 0;
			}
			break;

			default:
			{
				value = 0;
			}
			break;
		}
	}

	(void)elementNumber;
	// printf("TC value response: element=%u DDI=%u value=%ld\n", elementNumber, DDI, static_cast<long>(value));
	return true;
}

bool LemkenZirkon12DDOP::value_command_callback(std::uint16_t elementNumber,
                                                  std::uint16_t DDI,
                                                  std::int32_t processVariableValue,
                                                  void *parentPointer)
{
	// printf("TC value command: element=%u DDI=%u value=%ld\n", elementNumber, DDI, static_cast<long>(processVariableValue));
	if (nullptr != parentPointer)
	{
		auto sim = reinterpret_cast<LemkenZirkon12DDOP *>(parentPointer);

		// SetpointWorkState is nominally settable by the TC/task, but on this simple
		// rigid harrow the actual work state always tracks PTO engagement regardless.
		(void)sim;
		(void)DDI;
		(void)processVariableValue;
	}

	(void)elementNumber;
	return true;
}