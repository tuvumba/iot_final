Final project for BI-IOT
Keypad with password and IR activation for M5Stack Fire

Used hardware: 
  M5Stack Fire 2.6
  PIR unit AS312
  Keyboard based on MEGA328P
  
Consists of two parts:
  1) Simple TCP server
     A barebones python TCP server which listens for a connection and reads messages
  3) Code for M5STack
     Upon detecting a signal on PIR sensor, connects to a server and waits for user to enter the password![photo_2023-12-25_19-12-11](https://github.com/tuvumba/iot_final/assets/116572731/e7ba1d4d-f2f6-4042-8328-ce290b6bbf81)
