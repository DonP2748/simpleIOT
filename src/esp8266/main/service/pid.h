#ifndef _PID_H_
#define _PID_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	pid.h  												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	14/08/2023											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
#include "config.h"
//---------------------------------------//

//---------------MACRO-------------------//

#define EXAMPLE_TIM 		500
#define EXAMPLE_TAU  		5000
#define EXAMPLE_KP			0.5f
#define EXAMPLE_KI  		0.1f
#define EXAMPLE_KD			0.2f
#define EXAMPLE_LIMMIN 		0.0f
#define EXAMPLE_LIMMAX  	0.0f
#define EXAMPLE_LIMMININT   0.0f	
#define EXAMPLE_LIMMAXINT 	0.0f

//---------------------------------------//
//--------------DECLARE------------------//

typedef struct {

	/* Controller gains */
	float Kp;
	float Ki;
	float Kd;

	/* Derivative low-pass filter time constant */
	float tau;

	/* Output limits */
	float limMin;
	float limMax;
	
	/* Integrator limits */
	float limMinInt;
	float limMaxInt;

	/* Sample time (in seconds) */
	float T;

	/* Controller "memory" */
	float integrator;
	float prevError;			/* Required for integrator */
	float differentiator;
	float prevMeasurement;		/* Required for differentiator */

	/* Controller output */
	float out;

} PIDController;

PIDController *PIDController_Create(float kp,float ki,float kd,float tau,\
			float limmin,float limmax,float limmint,float limmaxt,float time);
void PIDController_Delete(PIDController *pid);
float PIDController_Update(PIDController *pid, float setpoint, float measurement,bool check_lim);
//---------------------------------------//
#endif//_PID_H_