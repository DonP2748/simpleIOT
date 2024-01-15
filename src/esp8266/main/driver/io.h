#ifndef _IO_H_
#define _IO_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	io.h												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
//---------------------------------------//

//---------------MACRO-------------------//
//---------------------------------------//

//--------------DECLARE------------------//
void init_io_device(void);
void set_relay_value(bool relay);
bool get_relay_value(bool relay);
void register_isr_inc_cb(void(*callback)(void));
void register_isr_dec_cb(void(*callback)(void));
//---------------------------------------//
#endif//_IO_H_