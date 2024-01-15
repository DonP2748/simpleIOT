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
#include "button.h"
#include "sensor.h"
#include "alarm.h"
#include "schedule.h"
#include "sntp_server/sntp.h"
//---------------------------------------//

//---------------GLOBAL------------------//
#define ON 				1
#define OFF  			0

#define MAXSIZE_LCD	16+1
typedef enum week {Sun, Mon, Tue, Wed, Thur, Fri, Sat} week;
//---------------------------------------//

//--------------PRIVATE------------------//
static sensor_t now = {0};
static date_t calendar = {0};
static date_t* time_sched = NULL;
static schedule_t* lc_sched = NULL;
static char* get_dow(int day);
static char* get_state(bool state);
static void show_date_time_lcd(bool arg);
static void show_schedule_lcd(void);
static void (*schedule_times_up_callback)(bool arg);
static bool pre_mode = 0;
static const char *TAG = "SCHEDULE";
//---------------------------------------//

schedule_t* init_schedule (void)
{
	init_lcd();
	//schedule_showoff_data(MODE_DATETIME);
	ESP_LOGI(TAG,"Init Schedule Module!");
	
	time_sched = (date_t*)calloc(1,sizeof(date_t));
	if(!time_sched) return NULL;
	lc_sched = (schedule_t*)calloc(1,sizeof(schedule_t));
	if(!lc_sched) return NULL;

	lc_sched->time = time_sched;
	return lc_sched;
}

void schedule_check_times_up(int warning)
{
	if(!lc_sched->state)
	{
		return ;
	}
	if(warning) return;
	if(calendar.dow == lc_sched->time->dow)
	{
		if(calendar.day == lc_sched->time->day)
		{
			if(calendar.month == lc_sched->time->month)
			{
				if(calendar.year == lc_sched->time->year)
				{
					if(calendar.hour == lc_sched->time->hour)
					{	
						if(calendar.minute == lc_sched->time->minute)
						{
							if(calendar.second == lc_sched->time->second)
							{
								schedule_times_up_callback(&(lc_sched->relay));
							}
						}
					}
				}
			}
		}
	}
	return ;
}

void schedule_update_realtime (void)
{
	struct tm* time_update = get_datetime();
	if(time_update == NULL) return;
	calendar.year = time_update->tm_year + 1900;//offset year la 1900
	calendar.month = time_update->tm_mon + 1;	//range tu 0-11
	calendar.day = time_update->tm_mday;		//range tu 1-31 ??
	calendar.dow = time_update->tm_wday;		//ngay dau tien la chu nhat = 0
	calendar.hour = time_update->tm_hour;		//may cai time nay khong biet co sai k nua
	calendar.minute = time_update->tm_min;
	calendar.second = time_update->tm_sec;
}

void schedule_showoff_data(void* arg)
{
	bool* type = (bool*)arg;
	if(type)
	{
		show_schedule_lcd();
		pre_mode = MODE_SCHEDULE;
	}
	else
	{
		show_date_time_lcd(pre_mode);
		pre_mode = MODE_DATETIME;
	} 

}

void register_schedule_times_up_cb (void(*callback)(bool arg))
{
	if(callback)
		schedule_times_up_callback = callback;
}

static void show_schedule_lcd(void)
{
	char* buff = (char*)calloc(MAXSIZE_LCD,sizeof(char));
	if(!buff) return;

	home();
	clear();
	setCursor(0,0);
	char* state = get_state(lc_sched->state);
	char* relay = get_state(lc_sched->relay);
	sprintf(buff,"Sched:%s",state);
	printstr(buff);
	setCursor(10,0);
	sprintf(buff,"RL:%s",relay);
	printstr(buff);
	setCursor(0,1);
	sprintf(buff,"  TIME: %02d:%02d  ",lc_sched->time->hour,lc_sched->time->minute);
	printstr(buff);
	free(buff);
}

static void show_date_time_lcd(bool arg)
{
	char* buff = (char*)calloc(MAXSIZE_LCD,sizeof(char));
	if(!buff) return;

	char* dow = get_dow(calendar.dow);
	if(arg)
	{
		home();
		clear();
	}
	setCursor(0,0);
	sprintf(buff,"%s, %02d-%02d-%04d",dow,calendar.day,calendar.month,calendar.year);
	printstr(buff);

	sensor_t* data = get_sensor_data_device();
	if((now.temp != data->temp)||(now.humi != data->humi)||(arg))
	{
		now.temp = data->temp;
		now.humi = data->humi;
		sprintf(buff,"T:%02d H:%02d",data->temp,data->humi);
		printf("%s\n",buff );
		setCursor(0,1);
		printstr(buff);
	}
	free(buff);
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