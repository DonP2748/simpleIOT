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
//---------------------------------------//

//--------------DECLARE------------------//
void init_device_process(void);
void process_alarm(void* arg);
void process_schedule(void* arg);
void process_send_response(void* arg);
//---------------------------------------//
#endif//_PROCESS_H_