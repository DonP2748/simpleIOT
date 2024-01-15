//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	button.c											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdlib.h>
#include <sys/unistd.h>
#include <stdio.h>
#include <string.h>

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "timer/systimer.h"
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "timer/systimer.h"


#include "io.h"
#include "schedule.h"
#include "button.h"
//---------------------------------------//

//---------------GLOBAL------------------//
//---------------------------------------//

//--------------PRIVATE------------------//
static relay_t* relay_output = NULL;
static bool mode_lcd = MODE_DATETIME;
static void button_relay_change_handler(void);
static void button_change_mode_handler(void);

static void (*relay_btn_change_callback)(bool arg);
static void (*mode_btn_change_callback)(void* arg); 
//---------------------------------------//


relay_t* init_button(void)
{
	init_io_device();
	register_isr_mode_cb(button_change_mode_handler);
	register_isr_relay_cb(button_relay_change_handler);

	relay_output = (relay_t*)calloc(1,sizeof(relay_t));
	if(!relay_output) return NULL;
	return relay_output;
}

void register_relay_btn_change_cb(void(*callback)(bool arg))
{
	if(callback)
		relay_btn_change_callback = callback;
}

void register_mode_btn_change_cb(void(*callback)(void* arg))
{
	if(callback)
		mode_btn_change_callback = callback;
}

bool get_lcd_showoff_mode(void)
{
	return mode_lcd;
}

static void button_relay_change_handler(void)
{
	relay_output->value = !relay_output->value;
	relay_btn_change_callback(relay_output->value);
}

static void button_change_mode_handler(void)
{
	mode_lcd = !mode_lcd;
	start_timer(100,mode_btn_change_callback,((mode_lcd) ? (&mode_lcd) : NULL));
}