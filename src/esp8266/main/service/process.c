//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	process.c											//
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
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "timer/systimer.h"
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "ota/ota.h"
#include "util/event.h"
#include "user_config.h"
#include "jsonparse.h"
#include "sntp/sntp.h"
#include "mqtt/mqtt.h"
#include "tcp/tcp.h"
#include "wifi/wifi.h"

#include "io.h"
#include "alarm.h"
#include "schedule.h"
#include "button.h"
#include "network.h"
#include "process.h"
#include "device.h"
//---------------------------------------//

//---------------GLOBAL------------------//
#define internal_device_t device_t 
//---------------------------------------//

//--------------PRIVATE------------------//

internal_device_t* device = NULL;

static void process_alarm(void*arg);
static void process_schedule(void* arg);
static void process_send_response(void* arg);
static void send_data (uint8_t protocol, char* data);
static void relay_event_handler(bool data);
static void schedule_event_handler(schedule_t* sched);
static void alarm_event_handler(alarm_t* alarm);
static void reset_factory_event_handler(void);
static void reboot_event_handler(void);
static void change_wifi_event_handler(device_info_t* info);
static void ota_event_handler(void);
static void device_info_event_handler(void);
static void update_sensor_event_handler(void);
static char* build_sending_msg(void);
static void clear_sending_msg(void);
static void process_data_recv_callback(void* data);

static const char* TAG = "PROCESS";
//---------------------------------------//

void init_device_process(void)
{
	ESP_LOGI(TAG,"Init Device Process !");
	REGISTER_EVENT(EVENT_DATA_SUM);
	device = init_local_device();
	register_alarm_over_threshold_cb(relay_event_handler);
	register_schedule_times_up_cb(relay_event_handler);
	register_relay_btn_change_cb(relay_event_handler);
	register_mode_btn_change_cb(schedule_showoff_data);
	
	register_process_data_json_cb(process_data_recv_callback);

	init_json_process();
	start_timer(100,process_alarm,NULL);
 	start_timer(100,process_schedule,NULL);
 	start_timer(100,process_send_response,NULL);
}


static char* build_sending_msg(void)
{
	create_json_msg();
	for(int i = 0;i<EVENT_DATA_SUM;i++)
	{
		if(CHECK_EVENT_FLAG(i))
		{
			create_object_json(i,device);
			CLEAR_EVENT_FLAG(i);
		}
	}
	return get_json_msg();

}

static void clear_sending_msg(void)
{
	relese_json_msg();
}


static void process_data_recv_callback(void* data)
{
	internal_device_t* dev = (internal_device_t*) data;
	for(int i = 0;i<EVENT_DATA_SUM;i++)
	{
		if(CHECK_EVENT_FLAG(i))
		{
			switch(i)
			{
				case SCHEDULE_EVENT:
					schedule_event_handler(dev->sched);
					break;
				case ALARM_EVENT:
					alarm_event_handler(dev->alarm);
					break;
				case RESET_FACTORY_EVENT:
					reset_factory_event_handler();
					break;
				case REBOOT_EVENT:
					reboot_event_handler();
					break;
				case CHANGE_WIFI_EVENT:
					change_wifi_event_handler(dev->info);
					break;
				case OTA_REQUEST_EVENT:
					ota_event_handler();
					break;
				case DEVICE_INFO_EVENT:
					device_info_event_handler();
					break;
				case CONTROL_RELAY_EVENT:
					relay_event_handler(dev->data->relay->value);
					break;
				case SENSOR_DATA_EVENT:
					update_sensor_event_handler();
					break;
				default:
					break;
			}
		}
	}
}

static void schedule_event_handler(schedule_t* arg)
{
	schedule_t* sched = arg;
	device->sched->time->year   = sched->time->year;
	device->sched->time->month  = sched->time->month;
	device->sched->time->day    = sched->time->day;
	device->sched->time->dow    = sched->time->dow; 
	device->sched->time->hour   = sched->time->hour;
	device->sched->time->minute = sched->time->minute;
	device->sched->time->second = sched->time->second;
	device->sched->state 		= sched->state;
	device->sched->repeat 		= sched->repeat;
	device->sched->relay 		= sched->relay;
	
}

static void alarm_event_handler(alarm_t* arg)
{
	alarm_t* alarm = arg;
	device->alarm->data->temp = alarm->data->temp;
	device->alarm->data->humi = alarm->data->humi;
	device->alarm->state      = alarm->state;
	device->alarm->status      = alarm->status;
}

static void reset_factory_event_handler(void)
{
	ESP_LOGI(TAG,"RESET FACTORY");
	esp_wifi_stop();
	esp_wifi_restore();
	esp_restart();
}

static void reboot_event_handler(void)
{
	ESP_LOGI(TAG,"REBOOT NOW");
	esp_restart();
}

static void change_wifi_event_handler(device_info_t* arg)
{
	device_info_t* info = arg;
	wifi_config_t wifi_config;

    bzero(&wifi_config, sizeof(wifi_config_t));
    memcpy(wifi_config.sta.ssid, info->ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, info->pass, sizeof(wifi_config.sta.password));

    esp_wifi_disconnect();
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_connect();
    esp_restart();
}

static void ota_event_handler(void)
{
	ESP_LOGI(TAG,"Start OTA Process!");
	OTA_NOW();
}

// static void secure_key_event_handler(void)
// {
// 	//None
// }

static void device_info_event_handler(void)
{
	//None
	get_network_info(&(device->info->rssi),
					   device->info->mac,
					   device->info->ssid,
					 &(device->info->ip));
}

static void update_sensor_event_handler(void)
{
	//None
}

static void relay_event_handler(bool data)
{
	device->data->relay->value = data;
	set_relay_value(data);
}	

#define MQTT_PROTOCOL	0
#define TCP4_PROCOTOL	1
static void send_data (uint8_t protocol, char* data)
{
	if(!protocol)
	{
		mqtt_publish_data_on_topic(NULL,data);
	}
	else
		tcp_server_push_notify(data); 
}


static void process_send_response(void* arg)
{
	if((IS_EVENT_COME)&&(is_wifi_connected()))
	{
		bool type = (is_internet_connected()) ? MQTT_PROTOCOL : TCP4_PROCOTOL;
		char* data = build_sending_msg();
		send_data(type,data);
		clear_sending_msg();
	}
	start_timer(RESPS_INTERVAL,process_send_response,NULL);
}

static void process_schedule(void* arg)
{
 	schedule_update_realtime();
 	schedule_check_times_up(device->alarm->status);
 	start_timer(SCHED_INTERVAL,process_schedule,NULL);
}

static void process_alarm(void* arg)
{
	alarm_check_threshold();
	SET_EVENT_FLAG(SENSOR_DATA_EVENT);
	if(get_lcd_showoff_mode()==MODE_DATETIME)
	{
		schedule_showoff_data(MODE_DATETIME);
	}
	start_timer(ALARM_INTERVAL,process_alarm,NULL);
}