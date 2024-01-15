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
static date_t calendar = {0};
static schedule_t *sched_now = NULL;
static schedule_t *sched_nxt = NULL;
static schedule_t *lc_sched[MAX_SCHEDULE] = {0};
static void sort_schedule(void);
static schedule_t *get_next_schedule(uint8_t arg);
static char* get_dow(int day);
static char* get_state(bool state);
static const char *TAG = "SCHEDULE";
//---------------------------------------//

schedule_t* init_schedule (void)
{
	init_lcd();
	ESP_LOGI(TAG,"Init Schedule Module!");
	return sched_now;
}


void schedule_showoff_data(void* arg)
{
	static int threshold_tmp = 0;
	static int state_tmp = 0;
	char buff[MAXSIZE_LCD] = {0};
	char* dow = get_dow(calendar.dow);
	if(arg)
	{
		home();
		clear();
	}
	if((threshold_tmp != sched_now->value)|(state_tmp != sched_now->state))
	{
		setCursor(0,0);
		sprintf(buff,"THRESHOLD:%02d %s",sched_now->value,get_state(sched_now->state));
		printstr(buff);
	}

	sensor_t* data = get_sensor_data_device();
	if((now.temp != data->temp)||(now.humi != data->humi))
	{
		now.temp = data->temp;
		now.humi = data->humi;
		sprintf(buff,"T:%02d H:%02d",data->temp,data->humi);
		printf("%s\n",buff );
		setCursor(0,1);
		printstr(buff);
	}
}


void schedule_check_times_up(int warning)
{
	bool checked = false;
	struct tm* time_update = get_datetime();
	if(time_update == NULL) 
	{
		return;
	}
	if((time_update->tm_wday == sched_nxt->dow)&&\
	(SCHED_TIME(sched_nxt->hour,sched_nxt->minute) == SCHED_TIME(time_update->tm_hour,time_update->tm_min)))
	{
		if(checked == false)
		{
			sched_now = sched_nxt;
			sched_nxt = get_next_schedule(sched_now->index);
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
	{
		if((lc_sched[index]->dow == sched->dow)&&\
		(SCHED_TIME(lc_sched[index]->hour,lc_sched[index]->minute) == SCHED_TIME(sched->hour,sched->minute)))
		{
			free(lc_sched[index]); 	//unique
		}
		if(lc_sched[index] == NULL)
		{
			lc_sched[index] = (schedule_t*)malloc(sizeof(schedule_t));
			if(!lc_sched[index])
			{	
				return;	
			}
			memset(lc_sched[index],0,sizeof(schedule_t));

			lc_sched[index]->index = index;
			lc_sched[index]->dow = sched->dow;
			lc_sched[index]->hour = sched->hour;
			lc_sched[index]->minute = sched->minute;
			lc_sched[index]->value = sched->value;
			lc_sched[index]->state = sched->state;
			lc_sched[index]->repeat = sched->repeat;
			lc_sched[index]->relay = sched->relay;
			break;
		}
	}
	//sort here
	sort_schedule();
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


static void sort_schedule(void)
{
	int i,j;
	bool wrapped;
	//bubble sort
	for(i = 0; i < MAX_SCHEDULE-1; i++)
	{
		for(j = 0;j < MAX_SCHEDULE-i-1;j++)
		{
			if((lc_sched[j] != NULL)&&((lc_sched[j]->dow > lc_sched[j+1]->dow)||((lc_sched[j]->dow == lc_sched[j+1]->dow)&\
			(SCHED_TIME(lc_sched[j]->hour,lc_sched[j]->minute) > SCHED_TIME(lc_sched[j+1]->hour,lc_sched[j+1]->minute)))))
			{
				int temp = lc_sched[j]->index;
				lc_sched[j]->index = lc_sched[j+1]->index;
				lc_sched[j+1]->index = temp;
				wrapped = true;
			}
		}
		if(wrapped == false)
		{
			break;
		}
	}
}

static schedule_t *get_next_schedule(uint8_t arg)
{
	int i;
	int index = arg;
	if(index == MAX_SCHEDULE-1)
	{
		index = 0;
	}
	else 
	{
		index += 1;
	}
	for(i = 0; i < MAX_SCHEDULE; i++)
	{
		if((lc_sched[i] != NULL)&&(lc_sched[i]->index == index))
		{
			return lc_sched[i];
		}
	}
	return NULL;
}

static char* get_dow(int day)
{
	switch(day)
	{
		case Mon:
			return "Mon";
		case Tue:
			return "Tue";
		case Wed:
			return "Wed";
		case Thur:
			return "Thur";
		case Fri:
			return "Fri";
		case Sat:
			return "Sat";
		case Sun:
			return "Sun";
		default:
			return NULL;
			break;
	}
}

static char* get_state(bool state)
{
	if(state)
		return "ON";
	else 
		return "OFF"; 
}



