//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	alarm.c												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdlib.h>
#include <sys/unistd.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "timer/systimer.h"
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"

#include "io.h"
#include "button.h"
#include "sensor.h"
#include "alarm.h"
//---------------------------------------//

//---------------GLOBAL------------------//

//---------------------------------------//

//--------------PRIVATE------------------//
sensor_t* dht = NULL;
sensor_t* threshold = NULL;
alarm_t* lc_alarm = NULL;
static void(*alarm_over_threshold_callback)(bool arg);

static const char *TAG = "ALARM";
//---------------------------------------//

alarm_t* init_alarm(void)
{
	sensor_init();
	ESP_LOGI(TAG,"Init Alarm!");

	dht = (sensor_t*)calloc(1,sizeof(sensor_t));
	if(!dht) return NULL;
	threshold = (sensor_t*)calloc(1,sizeof(sensor_t));
	if(!threshold) return NULL;
	lc_alarm  = (alarm_t*)calloc(1,sizeof(alarm_t));
	if(!lc_alarm) return NULL;

	lc_alarm->data = threshold;
	return lc_alarm;
}

sensor_t* get_sensor_data_device(void)
{
	return dht;
}


void alarm_check_threshold(void)
{
	if(sensor_read_data(dht) != ESP_OK)
		return;
	if(lc_alarm->state)
	{
		if((dht->temp >= lc_alarm->data->temp)|(dht->humi >= lc_alarm->data->humi))
		{
			if(!lc_alarm->status)
			{
				lc_alarm->status = true;
				bool val = true;
				alarm_over_threshold_callback(val);
			}
		}
		else
			lc_alarm->status = false; 
	}
	
}

void register_alarm_over_threshold_cb(void(*callback)(bool arg))
{
	if(callback)
		alarm_over_threshold_callback = callback;
}