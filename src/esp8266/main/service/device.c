//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	device.c											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdlib.h>
#include <sys/unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "timer/systimer.h"
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "io.h"
#include "device.h"
//---------------------------------------//

//---------------GLOBAL------------------//
//---------------------------------------//

//--------------PRIVATE------------------//
static app_data_t* lc_app_data = NULL;
static device_info_t* lc_device_info = NULL;
static device_t* local_device = NULL;
static device_info_t* init_device_info(void);
static app_data_t* init_app_data(void);
static const char* TAG = "DEVICE";
//---------------------------------------//


device_t* init_local_device(void)
{

	ESP_LOGI(TAG,"Init Local Device");
	local_device = (device_t*)malloc(sizeof(device_t));
	if(!local_device) return NULL;
	memset(local_device,0,sizeof(device_t));

	local_device->info  = init_device_info();
	local_device->sched = init_schedule();
	local_device->alarm = init_alarm();
	local_device->data  = init_app_data();
	return local_device;
}

static device_info_t* init_device_info(void)
{
	lc_device_info = (device_info_t*)malloc(sizeof(device_info_t));
	if(!lc_device_info) return NULL;
	memset(lc_device_info,0,sizeof(device_info_t));

	lc_device_info->id = 76543210;
	lc_device_info->hard_ver = 1;
	lc_device_info->soft_ver = 1;
	//lc_device_info->rssi = ;
	//lc_device_info->secure_key = ;
	//lc_device_info->mac = ;
	//lc_device_info->ssid = ;
	//lc_device_info->pass = ;
	//lc_device_info->ip = ;
	return lc_device_info;
}

static app_data_t* init_app_data(void)
{
	init_io_device();
	lc_app_data = (app_data_t*)malloc(sizeof(app_data_t));
	if(!lc_app_data) return NULL;
	memset(lc_app_data,0,sizeof(app_data_t));
	lc_app_data->sensor = get_sensor_data_device();
	return lc_app_data;
}