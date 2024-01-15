#ifndef _SYS_TIMER_H
#define _SYS_TIMER_H

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	systimer.h											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
#include <stdio.h>
#include <stdbool.h>
//---------------------------------------//
//---------------MACRO-------------------//
typedef void(*EventTimerCallback)(void* data);
//---------------------------------------//
//--------------DECLARE------------------//
void start_timer(uint32_t millis, EventTimerCallback callback, void* ptr);
void cancel_timer(EventTimerCallback callback);
bool is_timer_running(EventTimerCallback callback);
//---------------------------------------//
#endif//_SYS_TIMER_H