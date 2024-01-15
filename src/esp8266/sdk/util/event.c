//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	event.c												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <event.h>
#include <string.h>
//---------------------------------------//

//---------------GLOBAL------------------//
#define MAX_EVENT	32
//---------------------------------------//

//--------------PRIVATE------------------//
static uint32_t event_register;
//---------------------------------------//

void clear_event_flag(uint8_t evt)
{
	((event_register) &= (~(1 << (evt))));
}

void set_event_flag(uint8_t evt)
{
	((event_register) |= (1 << (evt)));
}

bool check_event_flag(uint8_t evt)
{
	return ((event_register) & ((uint32_t)1 << (evt)));
}

void clear_all_event(void)
{
	event_register = 0;
}

bool is_event_come(void)
{
	return (event_register) ? true : false;
}