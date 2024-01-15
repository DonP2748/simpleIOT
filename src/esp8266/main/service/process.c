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
#include "sensor.h"
#include "pid.h"
#include "io.h"
#include "sig_pwm.h"
#include "alarm.h"
#include "schedule.h"
#include "network.h"
#include "process.h"
#include "device.h"
//---------------------------------------//

//---------------GLOBAL------------------//
#define RESPS_INTERVAL  	25
#define SCHED_INTERVAL  	1000
#define ALARM_INTERVAL		5000
#define internal_device_t 	device_t 
#define MQTT_PROTOCOL		0
#define TCP4_PROCOTOL		1
//---------------------------------------//

//--------------PRIVATE------------------//
internal_device_t* device = NULL;
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
static void button_increase_handler(void);
static void button_decrease_handler(void);
static const char* TAG = "PROCESS";
//---------------------------------------//

void init_device_process(void)
{
	device = init_local_device();
	register_alarm_over_threshold_cb(relay_event_handler);
	register_process_data_json_cb(process_data_recv_callback);
	register_isr_inc_cb(button_increase_handler);
	register_isr_dec_cb(button_decrease_handler);
	init_json_process();
	ESP_LOGI(TAG,"Init Device Process !");
}



void process_alarm(void* arg)
{
	while(1)
	{
		alarm_check_threshold();
		SET_EVENT_FLAG(SENSOR_DATA_EVENT);
		vTaskDelay(ALARM_INTERVAL/portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

void process_schedule(void* arg)
{
 	while(1)
	{
		schedule_showoff_data(0);
 		schedule_check_times_up(device->alarm->status);
		vTaskDelay(SCHED_INTERVAL/portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

void process_send_response(void* arg)
{
	while(1)
	{
		if((IS_EVENT_COME)&&(is_wifi_connected()))
		{	
			bool type = (is_internet_connected()) ? MQTT_PROTOCOL : TCP4_PROCOTOL;
			char* data = build_sending_msg();
			send_data(type,data);
			clear_sending_msg();
		}
		vTaskDelay(RESPS_INTERVAL/portTICK_RATE_MS);
	}
	vTaskDelete(NULL);
}

#define EXAMPLE_MAX_PID_OUT 	(1800*10/2)

void process_control_power(void* arg)
{
	PIDController *pid = PIDController_Create(EXAMPLE_KP,EXAMPLE_KI,EXAMPLE_KD,EXAMPLE_TAU,\
				EXAMPLE_LIMMIN,EXAMPLE_LIMMAX,EXAMPLE_LIMMININT,EXAMPLE_LIMMAXINT,EXAMPLE_TIM);

	sensor_t *dht = get_sensor_data_device();
	uint8_t percent = 0;

	sig_pwm_init();

	while(1)
	{	
		sensor_read_data(dht);		
		float setpoint = (float)device->sched->value;
		PIDController_Update(pid,setpoint,dht->ftemp,false); 
//		ESP_LOGI(TAG,"PID OUT SIGNAL : %d",(int)pid->out);
		ESP_LOGI(TAG,"TEMP: %d HUMI: %d SETPOINT: %d",(int)dht->ftemp,(int)dht->fhumi,(int)setpoint);

		//example for pid, need specific algorithms for each specific case
		//assuming that default threshold need 50% to maintain
		//if temparature is higher then need more power to cool it down and vice versa

		percent = (uint8_t)((pid->out)/EXAMPLE_MAX_PID_OUT + 50); 
		sig_pwm_set_percent(percent);
		vTaskDelay(EXAMPLE_TIM/portTICK_RATE_MS);
	}
	PIDController_Delete(pid);
	vTaskDelete(NULL);
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
					relay_event_handler(dev->data->relay);
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


static void button_increase_handler(void)
{
	// Do not call ESP_LOGI in there, It will be crashed
	device->sched->value++;
}

static void button_decrease_handler(void)
{
	// Do not call ESP_LOGI in there, It will be crashed
	device->sched->value--; 
}

static void schedule_event_handler(schedule_t* arg)
{
	//do something... example
	schedule_t *sched = arg;
	schedule_create(sched);
	//should call schedule_remove() somewhere
}

static void alarm_event_handler(alarm_t* arg)
{
	alarm_t* alarm = arg;
	device->alarm->data->temp = alarm->data->temp;
	device->alarm->data->humi = alarm->data->humi;
	device->alarm->state      = alarm->state;
	device->alarm->status     = alarm->status;
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
	device->data->relay = data;
	set_relay_value(data);
}	

static void send_data (uint8_t protocol, char* data)
{
//	if(protocol == MQTT_PROTOCOL)
//	{
//		mqtt_publish_data_on_topic(NULL,data);
//	}
//	else if(protocol == TCP4_PROCOTOL)
//	{
	 	tcp_server_push_notify(data); 
//	}
}


