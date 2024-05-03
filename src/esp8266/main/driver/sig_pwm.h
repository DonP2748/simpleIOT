#ifndef _SIG_PWM_H_
#define _SIG_PWM_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	sig_pwm.h											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	0701/2023											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//---------------------------------------//

//---------------MACRO-------------------//
//---------------------------------------//

//--------------DECLARE------------------//
bool sig_pwm_init(void);
void sig_pwm_set_percent(uint8_t arg);
uint8_t sig_pwm_get_percent(void);
//---------------------------------------//
#endif//_SIG_PWM_H_