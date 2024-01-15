#ifndef _BUTTON_H_
#define _BUTTON_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	button.h											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
#include <stdbool.h>
//---------------------------------------//

//---------------MACRO-------------------//
#define MODE_DATETIME		0
#define MODE_SCHEDULE		1

typedef struct relay_t
{
	bool value;
} relay_t;
//---------------------------------------//

//--------------DECLARE------------------//
relay_t* init_button(void);
bool get_lcd_showoff_mode(void);
void register_relay_btn_change_cb(void(*callback)(bool arg));
void register_mode_btn_change_cb(void(*callback)(void* arg));
//---------------------------------------//
#endif//_BUTTON_H_