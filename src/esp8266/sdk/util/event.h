#ifndef _EVENT_H_
#define _EVENT_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	name.h												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
//---------------------------------------//

//---------------MACRO-------------------//
#define REGISTER_EVENT(e) 		init_register_event(e)
#define CLEAR_EVENT_FLAG(e)		clear_event_flag(e)
#define SET_EVENT_FLAG(e) 		set_event_flag(e)
#define CHECK_EVENT_FLAG(e)		check_event_flag(e)
#define IS_EVENT_COME			is_event_come()
#define CLEAR_ALL_EVENT			clear_all_event()
//---------------------------------------//
//--------------DECLARE------------------//
bool init_register_event(uint8_t evt_sum);
void clear_event_flag(uint8_t evt);
void set_event_flag(uint8_t evt);
bool check_event_flag(uint8_t evt);
void clear_all_event(void);
bool is_event_come(void);
//---------------------------------------//
#endif//_EVENT_H_