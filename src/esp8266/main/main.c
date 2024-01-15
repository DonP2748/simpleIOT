//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	main.c												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "timer/systimer.h"
#include <stdio.h>
#include "service/process.h"
#include "service/network.h"
#include "user_config.h"
#include "sig_pwm.h"
//---------------------------------------//

//---------------GLOBAL------------------//
static const char *TAG = "MAIN";

//---------------------------------------//


//--------------PRIVATE------------------//
//---------------------------------------//


void app_main()
{
    int a = 0;
    nvs_flash_init();
    esp_task_wdt_reset();
//    init_device_process();
//    init_network_process();

    while (1) 
    {
        // usleep(53);    
        // ets_delay_us(10);
        esp_task_wdt_reset();
        vTaskDelay(1/portTICK_RATE_MS);
    }
}

