#ifndef _SENSOR_H_
#define _SENSOR_H_
#ifdef __cplusplus
extern "C" {
#endif
//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	sensor.h											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <driver/gpio.h>
#include <esp_err.h>
//---------------------------------------//

//---------------MACRO-------------------//
typedef struct sensor_t
{
	int temp;
	int humi;
} sensor_t;
//---------------------------------------//

//--------------DECLARE------------------//
void sensor_init(void);
esp_err_t sensor_read_data(sensor_t* data);




#ifdef __cplusplus
}
#endif
//---------------------------------------//
#endif//_SENSOR_H_