//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	jsonparse.c											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tcpip_adapter.h"
#include "cJSON.h"
#include "util/event.h"
#include "user_config.h"
#include "jsonparse.h"
#include "esp_log.h"

#include "alarm.h"
#include "schedule.h"

#include "tcp/tcp.h"
#include "mqtt/mqtt.h"
#include "device.h"
//---------------------------------------//

//---------------GLOBAL------------------//
//---------------------------------------//

//--------------PRIVATE------------------//
static cJSON* device_json;
static cJSON* object_json;
static cJSON* element_json;
static cJSON* attribute_json;
static const char *TAG = "JSONPARSE";
static void (*process_data_json_callback)(void* data) = NULL;

static void parsing_data_receive_json(char* data);

static void create_schedule_object(schedule_t* sched);
static void create_alarm_object(alarm_t* alarm);
// static void create_reset_factory_object(void);
// static void create_reboot_object(void);
// static void create_change_wifi_object(device_info_t* dev_info);
// static void create_secure_key_object(char* key);
// static void create_ota_object(void);
static void create_device_info_object(device_info_t* dev_info);
static void create_relay_object(uint8_t data);
static void create_sensor_realtime_object(sensor_t* data);

//---------------------------------------//


void init_json_process(void)
{
	ESP_LOGI(TAG,"Init JSON Process");
	mqtt_data_handler_register(parsing_data_receive_json);
	tcp_data_handler_register(parsing_data_receive_json);
}

void create_json_msg(void)
{
	device_json = cJSON_CreateObject();
	//ESP_LOGI(TAG,"Create new json messenger!");
}
void relese_json_msg(void)
{    
	if(attribute_json != NULL)
	{
    	cJSON_Delete(attribute_json);
	}
    cJSON_Delete(element_json);
    cJSON_Delete(object_json);
    cJSON_Delete(device_json);

}

char* get_json_msg(void)
{
	return cJSON_Print(device_json);
}

void create_object_json (uint8_t evt, void* dev)
{
	device_t* create_dev = (device_t*)dev;
	switch(evt)
	{
		 case SCHEDULE_EVENT:
		 	create_schedule_object(create_dev->sched);
			break;
		case ALARM_EVENT:
			create_alarm_object(create_dev->alarm);
			break;
		// case RESET_FACTORY_EVENT:
		// 	create_reset_factory_object();
		// 	break;
		// case REBOOT_EVENT:
		// 	create_reboot_object();
		// 	break;
		// case CHANGE_WIFI_EVENT:
		// 	create_change_wifi_object(&(create_dev->info));
		// 	break;
		// case OTA_REQUEST_EVENT:
		// 	create_ota_object();
		// 	break;
		case DEVICE_INFO_EVENT:
			create_device_info_object(create_dev->info);
			break;
		case CONTROL_RELAY_EVENT:
			create_relay_object(create_dev->data->relay);
			break;
		case SENSOR_DATA_EVENT:
			create_sensor_realtime_object(create_dev->data->sensor);
			break;
		default:
			break;
	}
}

void register_process_data_json_cb(void(*callback)(void* data))
{
	if(callback)
	{
		process_data_json_callback = callback;
	}

}


static void parsing_data_receive_json(char* data)
{
	cJSON *dev = cJSON_Parse(data);
	cJSON *object; 
	cJSON *element;
	device_t *device = (device_t*)malloc(sizeof(device_t));
	if(!device)	return;
	if(cJSON_HasObjectItem(dev,"schedule"))
	{
		schedule_t sched = {0};
		device->sched = &sched;
		SET_EVENT_FLAG(SCHEDULE_EVENT);
		object = cJSON_GetObjectItem(dev,"schedule");

		if(cJSON_HasObjectItem(object,"dow"))
		{	
			device->sched->dow = atoi(cJSON_GetObjectItem(object,"dow")->valuestring);
		}
		if(cJSON_HasObjectItem(object,"hour"))
		{
			device->sched->hour = atoi(cJSON_GetObjectItem(object,"hour")->valuestring);
		}
		if(cJSON_HasObjectItem(object,"minute"))
		{
			device->sched->minute = atoi(cJSON_GetObjectItem(object,"minute")->valuestring);
		}

		if(cJSON_HasObjectItem(object,"value"))
		{
			device->sched->value = atoi(cJSON_GetObjectItem(object,"state")->valuestring);
		}
		if(cJSON_HasObjectItem(object,"state"))
		{
			device->sched->state = atoi(cJSON_GetObjectItem(object,"state")->valuestring);
		}
		if(cJSON_HasObjectItem(object,"repeat"))
		{
			device->sched->repeat = atoi(cJSON_GetObjectItem(object,"repeat")->valuestring);
		}
		if(cJSON_HasObjectItem(object,"relay"))
		{
			device->sched->relay = atoi(cJSON_GetObjectItem(object,"relay")->valuestring);
		}
		cJSON_Delete(object);
	}

	if(cJSON_HasObjectItem(dev,"alarm"))
	{
		alarm_t alarm = {0};
		sensor_t data = {0};
		device->alarm = &alarm;
		device->alarm->data = &data;
		SET_EVENT_FLAG(ALARM_EVENT);
		object = cJSON_GetObjectItem(dev,"alarm");
		if(cJSON_HasObjectItem(object,"data"))
		{
			element = cJSON_GetObjectItem(object,"data");
			if(cJSON_HasObjectItem(element,"temp"))
			{
				device->alarm->data->temp = atoi(cJSON_GetObjectItem(element,"temp")->valuestring);
			}
			if(cJSON_HasObjectItem(element,"humi"))
			{
				device->alarm->data->humi = atoi(cJSON_GetObjectItem(element,"humi")->valuestring);
			}
			cJSON_Delete(element);
		}
		if(cJSON_HasObjectItem(object,"state"))
		{
			device->alarm->data->temp = atoi(cJSON_GetObjectItem(object,"state")->valuestring);
		}
		if(cJSON_HasObjectItem(object,"status"))
		{
			device->alarm->data->humi = atoi(cJSON_GetObjectItem(object,"status")->valuestring);
		}	
		cJSON_Delete(object);		
	}
	if(cJSON_HasObjectItem(dev,"reset_factory"))
	{
		SET_EVENT_FLAG(RESET_FACTORY_EVENT);
	}
	if(cJSON_HasObjectItem(dev,"reboot"))
	{
		SET_EVENT_FLAG(REBOOT_EVENT);
	}
	if(cJSON_HasObjectItem(dev,"change_wifi"))
	{
		device_info_t info = {0};
		device->info = &info;
		SET_EVENT_FLAG(CHANGE_WIFI_EVENT);
		object = cJSON_GetObjectItem(dev,"change_wifi");
		if(cJSON_HasObjectItem(object,"ssid"))
		{
			memset(device->info->ssid,0,sizeof(device->info->ssid));
			strcpy(device->info->ssid,cJSON_GetObjectItem(object,"ssid")->valuestring);
		}
		if(cJSON_HasObjectItem(object,"pass"))
		{
			memset(device->info->pass,0,sizeof(device->info->pass));
			strcpy(device->info->pass,cJSON_GetObjectItem(object,"pass")->valuestring);
		}	
		cJSON_Delete(object);		
	}
	if(cJSON_HasObjectItem(dev,"ota"))
	{
		SET_EVENT_FLAG(OTA_REQUEST_EVENT);
	}
	// if(cJSON_HasObjectItem(dev,"secure_key"))
	// {
	// 	SET_EVENT_FLAG(SERCURE_KEY_EVENT);
	// }
	if(cJSON_HasObjectItem(dev,"device_info"))
	{
		SET_EVENT_FLAG(DEVICE_INFO_EVENT);
	}
	if(cJSON_HasObjectItem(dev,"relay"))
	{
		app_data_t data = {0};
		device->data = &data;
		SET_EVENT_FLAG(CONTROL_RELAY_EVENT);
		device->data->relay = atoi(cJSON_GetObjectItem(dev,"relay")->valuestring);
	}
	// if(cJSON_HasObjectItem(dev,"sensor"))
	// {
	// }
	process_data_json_callback(device);
	free(device);
	cJSON_Delete(dev);
}




static void create_schedule_object(schedule_t* sched)
{
	object_json = cJSON_AddObjectToObject(device_json,"schedule");
	char buffer[5] = {0};

    sprintf(buffer,"%d",sched->dow);
    element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "dow",element_json);

	sprintf(buffer,"%d",sched->hour);
    element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "hour",element_json);

	sprintf(buffer,"%d",sched->minute);
    element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "minute",element_json);

	sprintf(buffer,"%d",sched->value);
    element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "value",element_json);

	sprintf(buffer,"%d",sched->state);
    element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "state",element_json);

	sprintf(buffer,"%d",sched->repeat);
    element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "repeat",element_json);

	sprintf(buffer,"%d",sched->relay);
    element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "relay",element_json);
}

static void create_alarm_object(alarm_t* alarm)
{
	object_json = cJSON_AddObjectToObject(device_json, "alarm");
	char buffer[6] = {0};

	cJSON_AddItemToObject(object_json, "data",element_json);

	sprintf(buffer,"%02d",alarm->data->temp);
	attribute_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(element_json, "temp",attribute_json);

    sprintf(buffer,"%02d",alarm->data->humi);
	attribute_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(element_json, "humi",attribute_json);

	sprintf(buffer,"%d",alarm->state);
    element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "state",element_json);

    sprintf(buffer,"%d",alarm->status);
    element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "status",element_json);

}

// static void create_reset_factory_object(void)
// {
// 	object_json = cJSON_CreateString("OK");
//     cJSON_AddItemToObject(device_json, "reset_factory",object_json);
// }

// static void create_reboot_object(void)
// {
// 	object_json = cJSON_CreateString("OK");
//     cJSON_AddItemToObject(device_json, "reboot",object_json);
// }

// static void create_change_wifi_object(device_info_t* dev_info)
// {
// 	object_json = cJSON_AddObjectToObject(device_json, "change_wifi");
// 	element_json = cJSON_CreateString(dev_info->ssid);
//     cJSON_AddItemToObject(object_json, "ssid",element_json);

// 	element_json = cJSON_CreateString(dev_info->pass);
//     cJSON_AddItemToObject(object_json, "pass",element_json);
// }

// static void create_secure_key_object(char* key)
// {
// 	element_json = cJSON_CreateString(key);
//     cJSON_AddItemToObject(object_json, "secure_key",element_json);
// }

// static void create_ota_object(void)
// {
// 	object_json = cJSON_CreateString("OK");
//     cJSON_AddItemToObject(device_json, "ota_request",object_json);
// }

static void create_device_info_object(device_info_t* dev_info)
{
	object_json = cJSON_AddObjectToObject(device_json, "device_info");
	char* buffer = (char*)malloc(64*sizeof(char));
	if(!buffer)	return;
	memset(buffer,0,sizeof(char));

	sprintf(buffer,"%d",dev_info->id);
	element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "id",element_json);
    sprintf(buffer,"%d",dev_info->hard_ver);
	element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "hard_ver",element_json);
    sprintf(buffer,"%d",dev_info->soft_ver);
	element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "soft_ver",element_json);
    sprintf(buffer,"%d",dev_info->rssi);
    element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "rssi",element_json);
    
    // element_json = cJSON_CreateString(dev_info->secure_key);
    // cJSON_AddItemToObject(object_json, "secure_key",element_json);
    
    element_json = cJSON_CreateString(dev_info->mac);
    cJSON_AddItemToObject(object_json, "mac",element_json);
   
    element_json = cJSON_CreateString(dev_info->ssid);
    cJSON_AddItemToObject(object_json, "ssid",element_json);
    
    // element_json = cJSON_CreateString(dev_info->pass);
    // cJSON_AddItemToObject(object_json, "pass",element_json);

    ip4_addr_t ip;
    ip.addr = dev_info->ip;
    sprintf(buffer,IPSTR, IP2STR(&ip));
    element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "ip",element_json);

    free(buffer);	
}

static void create_relay_object(uint8_t data)
{
	object_json = cJSON_AddObjectToObject(device_json, "relay");
	char buffer[4] = {0};
	sprintf(buffer,"%d",data);
	element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "value",element_json);
}

static void create_sensor_realtime_object(sensor_t* data)
{
	object_json = cJSON_AddObjectToObject(device_json, "sensor");
	char buffer[6] = {0};
	sprintf(buffer,"%02d",data->temp);
	element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "temp",element_json);
    sprintf(buffer,"%02d",data->humi);
	element_json = cJSON_CreateString(buffer);
    cJSON_AddItemToObject(object_json, "humi",element_json);
}


