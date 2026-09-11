# DDOP-for-power-harrow

I am currently implementing an ISOBUS Task Controller Client using AgIsoStack++ for a LEMKEN Zirkon 12 power harrow. For that Task im using an ESP32-S3. 

The implement is a 3 m, rear-mounted power harrow with one working section. The intended functionality regarding the DDOP is relatively simple: If the PTO of the tractor is engaged the implement is working, if the PTO is not spinning the implement should be off/ not working. There isnt any logic to manually turn a section on or of.

I have been using the following resources as references:

- Task Controller Client tutorial: https://isobus-plus-plus.readthedocs.io/en/latest/Tutorials/Task_Controller_Client.html
- task_controller_client example: https://github.com/Open-Agriculture/AgIsoStack-plus-plus/tree/main/examples/task_controller_client
- section_control_implement_sim.cpp: https://github.com/Open-Agriculture/AgIsoStack-plus-plus/blob/main/examples/task_controller_client/section_control_implement_sim.cpp

I have also looked at the seeder implementation/example and tried to adapt the DDOP structure to the much simpler requirements of a power harrow.

**What I am unsure about**

I generated a DDOP-iop File , loaded it into the AgIsoStack DDOPGenerator but no error was shown. I would appreciate some guidance on the recommended/correct DDOP structure for this type of implement. I tested the Code on a LACOS LC ONE ISOBUS Terminal, the Terminal doesnt recognize a ISOBUS implement, the UT shows up without problems and i can control my relais via the ISOBUS Terminal.