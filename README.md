# simpleIOT

•  A simple project about Low cost IOT network with Thermostat and Linux pc

## Features

•  Network : support TCP (for LAN) and MQTT (for WAN) 
•  Function : remote control, schedule, alarm threshold, maintain temperature with pid
•  Others: OTA, security, storage, ...

## Installation

To install Papermark, you need to have esp8266-rtos (esp-idf for esp8266) installed on your system. Then, follow these steps:

•  Clone this repository: `git clone https://github.com/DonP2748/simpleIOT.git`

•  Enter the project directory: `cd simpleIOT/src/esp8266`

•  Build and flash project with esp-idf (after export): `idf.py build flash monitor`

•  Open Hive MQTT Websocket Client link : https://www.hivemq.com/demos/websocket-client/

•  Subcribe Topic : Thermostat_Transmit_[MAC Address] (topic that device will send data) 

•  Publish data on Topic : Thermostat_Receive_[MAC address] (topic that device will receive data) 

•  In LAN network, open linux terminal : telnet [LAN IP address] 3333 , device will auto choose MQTT if have internet connection and choose LAN if lose internet connection 

•  JSON message can be found in docs folder   


## Contributing

 welcomes contributions from anyone. If you have any questions, feedback, or suggestions, please feel free to contact me at donp172748@gmail.com.

## Support

•  If this project's helps or you like it, please leave it a Star. 

•  If you have any ideas about embedding, programming and need someone to work with, I'm happy to join.

