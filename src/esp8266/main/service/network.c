//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	network.c											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdlib.h>
#include <sys/unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <tcpip_adapter.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "timer/systimer.h"
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "mqtt/mqtt.h"
#include "tcp/tcp.h"
#include "wifi/wifi.h"
#include "device.h"

#include "sntp_server/sntp.h"
#include "network.h"
//---------------------------------------//

//---------------GLOBAL------------------//
#define MAX_WIFI_TIMEOUT		30000//5000 30000
#define MAX_NETWORK_TIMEOUT 	2000
//---------------------------------------//

//--------------PRIVATE------------------//
static void setup_wifi(void* arg);
static void setup_network(void* arg);
static void wifi_event_callback(int event);
static const char* TAG = "NETWORK";
//---------------------------------------//


void init_network_process(void)
{
	ESP_LOGI(TAG,"Init Network !");
	register_wifi_status_callback(wifi_event_callback);
	wifi_normal_start();
	start_timer(MAX_WIFI_TIMEOUT,setup_wifi,NULL);
	
}

void get_network_info(int* rssi,char* mac,char* ssid, uint32_t* ip)
{
	// rssi
	wifi_ap_record_t ap;
	esp_wifi_sta_get_ap_info(&ap);
	*rssi = ap.rssi;
	// printf("%d\n", ap.rssi);
	// mac
    esp_base_mac_addr_get((uint8_t*)mac);
    // ssid
    wifi_config_t wifi_config;
    bzero(&wifi_config, sizeof(wifi_config_t));
    esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config);
    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    // ip
	tcpip_adapter_ip_info_t ipInfo;
	tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
	*ip = ipInfo.ip.addr;

}

static void wifi_event_callback(int event)
{
	switch(event)
	{
		case WIFI_STATUS_DISCONNECTED:
			ESP_LOGW(TAG,"Wifi Disconnected!");
			start_timer(MAX_WIFI_TIMEOUT,setup_wifi,NULL);
			break;
		case WIFI_STATUS_CONNECTED:
			ESP_LOGI(TAG,"Wifi connected");
			cancel_timer(setup_wifi);
			break;
		case WIFI_STATUS_GOT_IP :
			ESP_LOGI(TAG,"Wifi Got IP");
			tcp_server_start();
			start_timer(MAX_NETWORK_TIMEOUT,setup_network,NULL);
			break;
		default:
			break;
	}
}

static void setup_wifi(void* arg)
{
	if(!is_wifi_connected())
	{
		ESP_LOGI(TAG,"Set Up Wifi");
		wifi_config_start(NULL);
	}
}


static void setup_network(void*arg)
{
	ESP_LOGI(TAG,"Set Up Network");
	mqtt_app_start();
	init_sntp_update();

}
