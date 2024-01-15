//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	systimer.c											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "driver/hw_timer.h"
#include "systimer.h"
#include "esp_log.h"
//---------------------------------------//
//---------------GLOBAL------------------//
#define TAG  "TIMERS"
#define TIMER_TYPE_SHORT        0
#define TIMER_TYPE_LONG         1
#define MAX_TIMER_COUNT         64

#define TIMER_EXPIRED(timer) ((uint32_t)(get_system_time_ms() - timer.start) >= timer.time)


//---------------------------------------//
//--------------PRIVATE------------------//
typedef struct sft_timer_t {
    uint32_t start;
    uint32_t time;
    void(*callback)(void* param);
    void* param;
} sft_timer_t;
static sft_timer_t timers[MAX_TIMER_COUNT] = { 0 };
static uint32_t get_system_time_ms(void);
//---------------------------------------//




void process_timer_events(void) {
    int index;
    static void* param = NULL;
    static void(*callback)(void* param) = NULL;

    for (index = 0; index < MAX_TIMER_COUNT; index++)
    {
        if ((timers[index].callback != NULL) && TIMER_EXPIRED(timers[index]))
        {
            callback = timers[index].callback;
            param = timers[index].param;
            timers[index].callback = NULL;
            callback(param);
        }
    }
}

void start_timer(uint32_t millis, void(*callback)(void* data), void* ptr) {
    int index;
    for (index = 0; index < MAX_TIMER_COUNT; index++)
    {
        if ((timers[index].callback == NULL) || (timers[index].callback == callback))
        {
            timers[index].start = get_system_time_ms();
            timers[index].callback = callback;
            timers[index].time = millis;
            timers[index].param = ptr;

            for (index = index + 1; index < MAX_TIMER_COUNT; index++) {
                if (timers[index].callback == callback)
                    timers[index].callback = NULL;
            }
            return;
        }
    }
}

void cancel_timer(void(*callback)(void* data)) {
    int index;
    for (index = 0; index < MAX_TIMER_COUNT; index++) {
        if (timers[index].callback == callback) {
            timers[index].callback = NULL;
            break;
        }
    }
}

bool is_timer_running(void(*callback)(void* data)) {
    int index;
    for (index = 0; index < MAX_TIMER_COUNT; index++) {
        if (timers[index].callback == callback)
            return true;
    }
    return false;
}

static uint32_t get_system_time_ms(void)
{
    return (uint32_t)(esp_timer_get_time()/1000);
}