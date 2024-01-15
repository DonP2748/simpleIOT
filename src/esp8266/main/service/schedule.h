#ifndef _SCHEDULE_H_
#define _SCHEDULE_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	schedule.h											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
//---------------------------------------//

//---------------MACRO-------------------//
typedef struct date_t 
{
	int year;
	int month;
	int day;
	int dow;
	int hour;
	int minute;
	int second;
} date_t;

typedef struct schedule_t
{
  	//void (*callback)(void* data);	
	date_t* time;
	bool state;
	bool repeat;
	bool relay;
} schedule_t;

//---------------------------------------//

//--------------DECLARE------------------//
schedule_t* init_schedule (void);
void schedule_showoff_data(void* arg);
void schedule_update_realtime(void);
void schedule_check_times_up(int warning);
void register_schedule_times_up_cb (void(*callback)(bool arg));
//---------------------------------------//
#endif//_SCHEDULE_H_