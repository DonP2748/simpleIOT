//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	schedule.c											//
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
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "timer/systimer.h"
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"


#include "lcd.h"
#include "sensor.h"
#include "alarm.h"
#include "schedule.h"
#include "sntp_server/sntp.h"
//---------------------------------------//

//---------------GLOBAL------------------//
#define ON 				1
#define OFF  			0
#define MAXSIZE_LCD		16+1
#define MAX_SCHEDULE 	8
#define SCHED_TIME(h,m) (uint32_t)(h*100+m)

typedef enum week {Sun, Mon, Tue, Wed, Thur, Fri, Sat} week;
//---------------------------------------//
//--------------PRIVATE------------------//
static sensor_t now = {0};
static schedule_t *sched_now = NULL;
static schedule_t *sched_next = NULL;
static schedule_t *lc_sched[MAX_SCHEDULE] = {0};
static schedule_t *get_next_schedule(uint8_t dow);
static void move_data_schedule(schedule_t *sched);
static const char *TAG = "SCHEDULE";
//---------------------------------------//

schedule_t* init_schedule (void)
{
	init_lcd();
	sched_now = (schedule_t *)malloc(sizeof(schedule_t));
	if(sched_now == NULL){ return NULL; }
	memset(sched_now,0,sizeof(schedule_t));	

	struct tm *time_update = get_datetime();
	if(time_update != NULL)
	{
		sched_now->dow    = time_update->tm_wday;
		sched_now->hour   = time_update->tm_hour;
		sched_now->minute = time_update->tm_min;
	}
	sched_now->value  = 25;
	sched_now->state  = true;
	sched_now->repeat = true;
	sched_now->relay  = false;

	ESP_LOGI(TAG,"Init Schedule Module!");
	sched_next = sched_now;
	return sched_now;
}


void schedule_showoff_data(void* arg)
{
	static int threshold_tmp = 0;
	char buff[MAXSIZE_LCD] = {0};
	if(arg)
	{
		home();
		clear();
	}
	if(threshold_tmp != sched_now->value)
	{
		setCursor(0,0);
		sprintf(buff,"THRESHOLD:%02d",sched_now->value);
		printstr(buff);
	}

	sensor_t* data = get_sensor_data_device();
	if((now.temp != data->temp)||(now.humi != data->humi))
	{
		now.temp = data->temp;
		now.humi = data->humi;
		sprintf(buff,"T:%02d H:%02d",data->temp,data->humi);
		//printf("%s\n",buff );
		setCursor(0,1);
		printstr(buff);
	}
}


void schedule_check_times_up(int warning)
{
	bool checked = false;
	struct tm* time_update = get_datetime();
	if(time_update == NULL){ return; }
	//get next sched here
	sched_next = get_next_schedule(time_update->tm_wday);
	//check null
	if(sched_next == NULL){	return; }

	if((time_update->tm_wday == sched_next->dow)&&\
	(SCHED_TIME(sched_next->hour,sched_next->minute) == SCHED_TIME(time_update->tm_hour,time_update->tm_min)))
	{	
		if(checked == false)
		{
			//move data to sched_now
			move_data_schedule(sched_next);			
			checked = true;
		}
	}
	else 
	{
		checked = false;
	}
}


void schedule_create(schedule_t *sched)
{
	int index;
	for(index = 0; index < MAX_SCHEDULE; index++)
	{	//check schedule existed
		if((lc_sched[index] != NULL)&&(lc_sched[index]->dow == sched->dow)&&\
		(SCHED_TIME(lc_sched[index]->hour,lc_sched[index]->minute) == SCHED_TIME(sched->hour,sched->minute)))
		{
			free(lc_sched[index]); 	//unique
		}
		if(lc_sched[index] == NULL)
		{
			lc_sched[index] = (schedule_t*)malloc(sizeof(schedule_t));
			if(!lc_sched[index]){ return; }
			memset(lc_sched[index],0,sizeof(schedule_t));

			lc_sched[index]->dow = sched->dow;
			lc_sched[index]->hour = sched->hour;
			lc_sched[index]->minute = sched->minute;
			lc_sched[index]->value = sched->value;
			lc_sched[index]->state = sched->state;
			lc_sched[index]->repeat = sched->repeat;
			lc_sched[index]->relay = sched->relay;
			return;
		}
	}
}

void schedule_delete(schedule_t *sched)
{
	int index;
	for(index = 0; index < MAX_SCHEDULE; index++)
	{
		if((lc_sched[index] != NULL)&&(lc_sched[index]->dow == sched->dow)&&\
		(SCHED_TIME(lc_sched[index]->hour,lc_sched[index]->minute) == SCHED_TIME(sched->hour,sched->minute)))
		{
			free(lc_sched[index]);
			return;
		}
	}
}


static schedule_t *get_next_schedule(uint8_t dow)
{
	uint8_t index;
	uint8_t location = 0; 
	uint16_t diff = 2359; //23h59'

	struct tm* time_update = get_datetime();
	if(time_update == NULL){ return NULL; } 

	for(index = 0; index < MAX_SCHEDULE; index++)
	{ 
		if((lc_sched[index] != NULL)&&(lc_sched[index]->state == true)&&(lc_sched[index]->dow == dow))
		{
			int16_t tmp =  SCHED_TIME(lc_sched[index]->hour,lc_sched[index]->minute) - \
							SCHED_TIME(time_update->tm_hour,time_update->tm_min);						
			if((tmp >= 0)&&(tmp < diff))
			{
				diff = tmp;
				location = index;
			}
		}
	}
	if(diff != 2359){ return lc_sched[location]; }
	else{ return NULL; } 
}



static void move_data_schedule(schedule_t *sched)
{
	sched_now->dow 		= sched->dow;
	sched_now->hour 	= sched->hour;
	sched_now->minute 	= sched->minute;
	sched_now->value 	= sched->value;
	sched_now->state 	= sched->state;
	sched_now->repeat 	= sched->repeat;
	sched_now->relay 	= sched->relay;	
	//Turn off if not repeat
	if(!sched->repeat)
	{
		sched->state = false;
	}
} 



