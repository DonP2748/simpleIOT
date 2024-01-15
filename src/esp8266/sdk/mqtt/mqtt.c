//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	mqtt.c												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_err.h"
#include "mqtt_client.h"
#include "mqtt.h"
//---------------------------------------//

//---------------GLOBAL------------------//
#define TAG  "MQTT"
#define EXAMPLE_BROKER_URL "mqtt://broker.hivemq.com"   // mqtt://broker.hivemq.com  
#define TOPIC_LENGTH_MAX  64
char *device_topic_recv = NULL;
char *device_topic_tran = NULL;
//---------------------------------------//

//--------------PRIVATE------------------//
static uint8_t internet_connected = false;
static esp_mqtt_client_handle_t client;
//static void (*mqtt_status_callback)(int32_t status) = NULL;
static void (*mqtt_data_handler_calback)(char *data) = NULL;
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event);
static void mqtt_connected_callback (void);
//---------------------------------------//


void mqtt_app_start(void)
{
    ESP_LOGI(TAG,"start mqtt client");
    esp_mqtt_client_config_t mqtt_cfg = 
    {
        .uri = EXAMPLE_BROKER_URL,//CONFIG_BROKER_URL
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}



void mqtt_data_handler_register(void (*callback)(char* data))
{
    if((callback != NULL) && (mqtt_data_handler_calback == NULL))
    {
        ESP_LOGI(TAG, "MQTT register data callback handler successfull");
        mqtt_data_handler_calback = callback;
        return;
    }
    ESP_LOGI(TAG, "MQTT register data callback handler failed");
    return;
}

void mqtt_data_handler_unregister(void)
{
    mqtt_data_handler_calback = NULL;
}

#define	LEN 	0 
#define	QoS		1	
#define	RETAIN 	0
void mqtt_publish_data_on_topic(char *topic, char *data)
{
    if(device_topic_tran)
        esp_mqtt_client_publish(client,(topic == NULL) ? device_topic_tran : topic, data, LEN, QoS, RETAIN);
}

void mqtt_subcribe_topic(char *topic)
{
    esp_mqtt_client_subscribe(client,topic,0);
}

bool is_internet_connected(void)
{
    return internet_connected ;   
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) 
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    //esp_mqtt_client_handle_t lc_client = event->client;
    // your_context_t *context = event->context;
    switch (event->event_id) 
    {
    	case MQTT_EVENT_ANY:
            break;
        case MQTT_EVENT_CONNECTED:
            internet_connected = true;
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqtt_connected_callback();
            break;
        case MQTT_EVENT_DISCONNECTED:
            internet_connected = false;
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            mqtt_data_handler_calback(event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        case MQTT_EVENT_BEFORE_CONNECT:
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}

static void mqtt_connected_callback (void)
{
    uint8_t mac[6];
    esp_read_mac(mac,ESP_MAC_WIFI_STA);
    device_topic_recv = (char*)malloc(40*sizeof(char));
    if (!device_topic_recv){
        ESP_LOGE(TAG,"Set Topic Failed");
        return;
    }
    sprintf(device_topic_recv,"Thermostat_Receive_"MACSTR,MAC2STR(mac));
    esp_mqtt_client_subscribe(client,device_topic_recv, 0);
    ESP_LOGI(TAG, "sent subscribe successful TOPIC : %s MAC : "MACSTR,device_topic_recv,MAC2STR(mac));
    device_topic_tran = (char*)malloc(40*sizeof(char));    
    if (!device_topic_tran){
        ESP_LOGE(TAG,"Set Topic Failed");
        return;
    }
    sprintf(device_topic_tran,"Thermostat_Transmit_"MACSTR,MAC2STR(mac));
    ESP_LOGI(TAG, "sent publish successful TOPIC : %s MAC : "MACSTR,device_topic_tran,MAC2STR(mac));
}