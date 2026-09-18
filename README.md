# DDOP-for-power-harrow

I am currently implementing an ISOBUS Task Controller Client using AgIsoStack++ for a LEMKEN Zirkon 12 power harrow. For that Task im using an ESP32-S3. 

The implement is a 3 m, rear-mounted power harrow with one working section. The intended functionality regarding the DDOP is relatively simple: If the PTO of the tractor is engaged the implement is working, if the PTO is not spinning the implement should be off/ not working. There isnt any logic to manually turn a section on or of.

I have been using the following resources as references:

- Task Controller Client tutorial: https://isobus-plus-plus.readthedocs.io/en/latest/Tutorials/Task_Controller_Client.html
- task_controller_client example: https://github.com/Open-Agriculture/AgIsoStack-plus-plus/tree/main/examples/task_controller_client
- section_control_implement_sim.cpp: https://github.com/Open-Agriculture/AgIsoStack-plus-plus/blob/main/examples/task_controller_client/section_control_implement_sim.cpp

I have also looked at the seeder implementation/example and tried to adapt the DDOP structure to the much simpler requirements of a power harrow.

**What I am unsure about**

I tried using the tine revolutions per square meter as process data, but its not working. The Terminal in the Tractor shwos, that the implement is working when the PTO is spinning, yet no colour gradient or something esle is shwon, indicating that the process data reaches its destination.