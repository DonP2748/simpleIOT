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

    init_device_process();
    init_network_process();

    xTaskCreate(process_alarm, "process_alarm", 4096, NULL, 3, NULL);
    xTaskCreate(process_schedule, "process_schedule", 4096, NULL, 3, NULL);
    xTaskCreate(process_send_response, "process_send_response", 4096, NULL, 3, NULL);
}

