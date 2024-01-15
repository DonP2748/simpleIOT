//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	systimer.c											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	06/01/2023											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
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
#define MAX_BLOCK_TICKS         100
typedef struct 
{
    TimerHandle_t xtimer;
    EventTimerCallback cb;
} SoftTimer_t;
//---------------------------------------//
//--------------PRIVATE------------------//
static SoftTimer_t  timers[MAX_TIMER_COUNT] = { 0 };
static void run_timer_callback(TimerHandle_t xTimer);
//---------------------------------------//

void start_timer(uint32_t millis, EventTimerCallback callback, void* ptr)
{
    int index;
    for (index = 0; index < MAX_TIMER_COUNT; index++)
    {
        if (timers[index].cb == callback)
        {
            xTimerChangePeriod(timers[index].xtimer,pdMS_TO_TICKS(millis),MAX_BLOCK_TICKS);
            return;
        }
        else if ((timers[index].xtimer != NULL)&&(xTimerIsTimerActive(timers[index].xtimer) == pdFALSE))
        {
            xTimerStop(timers[index].xtimer,MAX_BLOCK_TICKS);
            xTimerDelete(timers[index].xtimer,MAX_BLOCK_TICKS);
            timers[index].xtimer = NULL;
            timers[index].cb = NULL;
        }
        if (timers[index].xtimer == NULL)
        {
            timers[index].cb = callback;
            timers[index].xtimer = xTimerCreate("xTIMER",pdMS_TO_TICKS(millis),0,ptr,run_timer_callback);
            xTimerStart(timers[index].xtimer,MAX_BLOCK_TICKS);
            return;
        }
    }
}

void cancel_timer(EventTimerCallback callback) 
{
    int index;
    for (index = 0; index < MAX_TIMER_COUNT; index++) 
    {
        if (timers[index].cb == callback) 
        {
            timers[index].cb = NULL;
            xTimerStop(timers[index].xtimer,MAX_BLOCK_TICKS);
            xTimerDelete(timers[index].xtimer,MAX_BLOCK_TICKS);
            return;
        }
    }
}

bool is_timer_running(EventTimerCallback callback) 
{
    int index;
    for (index = 0; index < MAX_TIMER_COUNT; index++) 
    {
        if ((timers[index].cb == callback)&&(timers[index].xtimer != NULL))
        {
            int ret = xTimerIsTimerActive(timers[index].xtimer);
            if (ret != pdFALSE ) return true;
            else return false;
        }
    }
    return false;
}


static void run_timer_callback(TimerHandle_t xTimer)
{
    int index;
    EventTimerCallback callback = NULL;
    for (index = 0; index < MAX_TIMER_COUNT; index++) 
    {
        if (timers[index].xtimer == xTimer)
        {
            callback = timers[index].cb;
            callback(pvTimerGetTimerID(xTimer));
            //xTimerStop(timers[index].xtimer,MAX_BLOCK_TICKS);
            //xTimerDelete(timers[index].xtimer,MAX_BLOCK_TICKS);
            //timers[index].xtimer = NULL;
            //timers[index].cb = NULL;
            return;
        }
    }
}

/* CODE FAILED EXPLAINED
Neu dat start timer trong callback

void test_callback(void* data)
{
    static int count = 0;
    ESP_LOGI(TAG,"HELLO FKYou AAAAAAAAAAAAAAAAAAA %d",count);
    count += 1;
    start_timer(1000,test_callback,NULL);
}

voi ham run callback nhu nay
static void run_timer_callback(TimerHandle_t xTimer)
{
    int index;
    EventTimerCallback callback = NULL;
    for (index = 0; index < MAX_TIMER_COUNT; index++) 
    {
        if (timers[index].xtimer == xTimer)
        {
            callback = timers[index].cb;
            callback(pvTimerGetTimerID(xTimer)); //-> day chinh la ham test_callback, khi run ham nay lai start_timer o duoi nhay vao if vi callback van goi chinh no, tuc restart timer voi xTimerChangePeriod, luc nay xtimer van dang co callback la callback 
            //xTimerStop(timers[index].xtimer,MAX_BLOCK_TICKS);
            //xTimerDelete(timers[index].xtimer,MAX_BLOCK_TICKS);
            //timers[index].xtimer = NULL;
            timers[index].cb = NULL;   // Sau do  callback bi gan null va 1s sau run lai callback(pvTimerGetTimerID(xTimer)); thi thay callback mat me roi vi da bi gan null ?:D?, nen crash, co the fix bang cach them if(callback) check xem co callback k thi moi call. xTimerStop va xTimerDelete cung the, neu delete va gan Null cho xtimer xong ti nua run no thay xtimer null the la cancel luon, phai xac dinh duoc flow cuar func
            return;
        }
    }
}


void start_timer(uint32_t millis, EventTimerCallback callback, void* ptr)
{
    int index;
    for (index = 0; index < MAX_TIMER_COUNT; index++)
    {
        if (timers[index].cb == callback)
        {
            xTimerChangePeriod(timers[index].xtimer,pdMS_TO_TICKS(millis),MAX_BLOCK_TICKS);
            return;
        }
        else if ((timers[index].xtimer == NULL))
        { 
            timers[index].cb = callback;
            timers[index].xtimer = xTimerCreate("xTIMER",pdMS_TO_TICKS(millis),0,ptr,run_timer_callback);
            xTimerStart(timers[index].xtimer,MAX_BLOCK_TICKS);
            return;
        }
    }
}


//Co Ham BaseType_t xTimerPendFunctionCall(
                            PendedFunction_t xFunctionToPend,
                            void *pvParameter1,
                            uint32_t ulParameter2,
                            TickType_t xTicksToWait ); trong kha giong start timer voi 2 arg nhung khong co tinh nang cancel va check running, do k co xtimer de "tom'" vao




xTimerIsTimerActive(timers[index].xtimer) == pdFALSE chi dung cho timer da create, neu chua create thi crash
*/