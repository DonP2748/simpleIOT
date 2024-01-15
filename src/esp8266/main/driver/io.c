//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	relay.c												//
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
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "timer/systimer.h"

#include "io.h"
//---------------------------------------//

//---------------GLOBAL------------------//
#define ESP_INTR_FLAG_DEFAULT 	0

#define CONFIG_RELAY_PIN		12
#define GPIO_OUTPUT_PIN_SEL  	((1ULL<<CONFIG_RELAY_PIN))
#define CONFIG_DECREASE_PIN 	5 //
#define CONFIG_INCREASE_PIN		0 //
#define GPIO_INPUT_PIN_SEL  	((1ULL<<CONFIG_DECREASE_PIN)|(1ULL<<CONFIG_INCREASE_PIN))

#define DEBOUNCE_TIME			500


//---------------------------------------//

//--------------PRIVATE------------------//


static bool debounce_increase_btn = false; 
static bool debounce_decrease_btn = false;
static void debounce_increase_release(void* arg);
static void debounce_decrease_release(void* arg);
static void IRAM_ATTR decrease_button_isr_handler(void* arg);
static void IRAM_ATTR increase_button_isr_handler(void* arg);
static void(*button_increase_handler)(void) = NULL; 
static void(*button_decrease_handler)(void) = NULL; 
//---------------------------------------//


void set_relay_value(bool relay)
{
	gpio_set_level(CONFIG_RELAY_PIN, relay);
}

bool get_relay_value(bool relay)
{
	return gpio_get_level(CONFIG_RELAY_PIN);
}

void init_io_device(void)
{
	
	// configure button and led pins as GPIO pins
	gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);


    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
	


	// set the correct direction
	gpio_set_direction(CONFIG_INCREASE_PIN, GPIO_MODE_INPUT);
	gpio_set_direction(CONFIG_DECREASE_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(CONFIG_RELAY_PIN, GPIO_MODE_OUTPUT);
	
	// enable interrupt on falling (1->0) edge for button pin
	gpio_set_intr_type(CONFIG_INCREASE_PIN, GPIO_INTR_NEGEDGE);
	gpio_set_intr_type(CONFIG_DECREASE_PIN, GPIO_INTR_NEGEDGE);
	
	// install ISR service with default configuration
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	
	// attach the interrupt service routine
	gpio_isr_handler_add(CONFIG_INCREASE_PIN, increase_button_isr_handler, NULL); //
	gpio_isr_handler_add(CONFIG_DECREASE_PIN, decrease_button_isr_handler, NULL);
}

void register_isr_dec_cb(void(*callback)(void))
{
	if(callback)
		button_decrease_handler = callback;
}

void register_isr_inc_cb(void(*callback)(void))
{
	if(callback)
		button_increase_handler = callback;
}

static void debounce_decrease_release(void* arg)
{
	debounce_decrease_btn = false; 
}


static void debounce_increase_release(void* arg)
{
	debounce_increase_btn = false;
}


// interrupt service routine, called when the button is pressed
static void IRAM_ATTR increase_button_isr_handler(void* arg)
{
	
    // notify the button task
	// Do not call ESP_LOGI in there, and in callback too! It will be crashed
	if(!debounce_increase_btn)
	{
		debounce_increase_btn = true;
		start_timer(DEBOUNCE_TIME,debounce_increase_release,NULL);
		if(button_increase_handler)
		{
			button_increase_handler();
		}
	}
	
}

static void IRAM_ATTR decrease_button_isr_handler(void* arg)
{
	
	if(!debounce_decrease_btn)
	{
		debounce_decrease_btn = true;
		start_timer(DEBOUNCE_TIME,debounce_decrease_release,NULL);
		if(button_decrease_handler)
		{
			button_decrease_handler();
		}
	}
}