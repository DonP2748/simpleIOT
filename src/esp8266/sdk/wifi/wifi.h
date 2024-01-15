#ifndef _WIFI_H
#define _WIFI_H

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	wifi.h												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
//---------------------------------------//

//---------------MACRO-------------------//
#define WIFI_STATUS_DISCONNECTED    0
#define WIFI_STATUS_CONNECTED       1
#define WIFI_STATUS_GOT_IP          2

#define WIFI_CONFIG_EVENT_DONE		1

typedef void(*wifi_config_callback_t)(int event, void* arg);

//---------------------------------------//

//--------------DECLARE------------------//
bool is_wifi_connected(void);
void wifi_config_start(wifi_config_callback_t callback);
void register_wifi_status_callback(void(*callback)(int status));
//---------------------------------------//
#endif//_WIFI_H