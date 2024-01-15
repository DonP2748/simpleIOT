#ifndef _MQTT_H_
#define _MQTT_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	mqtt.h												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
//---------------------------------------//

//---------------MACRO-------------------//
//---------------------------------------//

//--------------DECLARE------------------//
void mqtt_app_start(void);
void mqtt_data_handler_register(void (*callback)(char* data));
void mqtt_data_handler_unregister(void);
void mqtt_publish_data_on_topic(char *topic, char *data);
void mqtt_subcribe_topic(char *topic);
bool is_internet_connected(void);
//---------------------------------------//
#endif//_MQTT_H_