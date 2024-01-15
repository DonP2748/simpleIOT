//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	pwm.c												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "esp8266/gpio_register.h"
#include "esp8266/pin_mux_register.h"

#include "driver/pwm.h"
#include "sig_pwm.h"
//---------------------------------------//

//---------------GLOBAL------------------//
#define PWM_1_OUT_IO_NUM   	13
#define PWM_PERIOD    		(1000)
#define NUMBERS_CHANNEL     1
#define CHANNEL_NUMB 		NUMBERS_CHANNEL-1
//---------------------------------------//

//--------------PRIVATE------------------//
static uint32_t duty = 0;
static uint32_t pin_num = PWM_1_OUT_IO_NUM;
static const char *TAG = "PWM SIGNAL";
//---------------------------------------//

bool sig_pwm_init(void)
{
	int ret;
	ret = pwm_init(PWM_PERIOD,  &duty, NUMBERS_CHANNEL, &pin_num);
	ret = pwm_set_phase(CHANNEL_NUMB,0);
	if (ret != ESP_OK) return false;
	return true;
} 

void sig_pwm_set_percent(uint8_t arg)
{
	uint8_t val = (arg > 100) ? 100 : ((arg < 0) ? 0 : arg);
	duty = val*(PWM_PERIOD/100);
	pwm_set_duty(CHANNEL_NUMB,duty);
    pwm_start();
}

uint8_t sig_pwm_get_percent(void)
{
	pwm_get_duty(CHANNEL_NUMB,&duty);
	return (duty*100)/PWM_PERIOD;
}