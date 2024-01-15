#ifndef _ALARM_H_
#define _ALARM_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	alarm.h												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
#include "sensor.h"
//---------------------------------------//

//---------------MACRO-------------------//
typedef struct alarm_t
{
	sensor_t* data;
	bool state;
	bool status;
} alarm_t;
//---------------------------------------//

//--------------DECLARE------------------//
alarm_t* init_alarm(void);
void alarm_check_threshold(void);
sensor_t* get_sensor_data_device(void);
void register_alarm_over_threshold_cb(void(*callback)(bool arg));
//---------------------------------------//
#endif//_ALARM_H_