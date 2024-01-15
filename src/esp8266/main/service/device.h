#ifndef _DEVICE_H_
#define _DEVICE_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	device.h											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
#include "alarm.h"
#include "schedule.h"
//---------------------------------------//

//---------------MACRO-------------------//
typedef struct device_info_t
{
	int id;
	int hard_ver;
	int soft_ver;
	int rssi;
	char secure_key[16];
	char mac[6];
	char ssid[32];
	char pass[64];
	uint32_t ip;
} device_info_t;

typedef struct app_data_t 
{
	bool			relay;
	sensor_t* 		sensor;
} app_data_t;

typedef struct device_t 
{
	alarm_t* 		alarm;
	app_data_t*		data;
	schedule_t* 	sched;
	device_info_t* 	info;
} device_t;


//---------------------------------------//

//--------------DECLARE------------------//
device_t* init_local_device(void);
//---------------------------------------//
#endif//_DEVICE_H_