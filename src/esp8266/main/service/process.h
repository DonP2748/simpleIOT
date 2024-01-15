#ifndef _PROCESS_H_
#define _PROCESS_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	process.h											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
//---------------------------------------//

//---------------MACRO-------------------//
#define RESPS_INTERVAL  25
#define SCHED_INTERVAL  1000
#define ALARM_INTERVAL	5000
//---------------------------------------//

//--------------DECLARE------------------//
void init_device_process(void);
void main_process_notify(void);
//---------------------------------------//
#endif//_PROCESS_H_