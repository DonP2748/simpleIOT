//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	pid.c											    //
// AUTHOR 	: 	DonP 												//
// DATE   	: 	14/08/2023											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "pid.h"
//---------------------------------------//

//---------------GLOBAL------------------//
//---------------------------------------//

//--------------PRIVATE------------------//
//---------------------------------------//


PIDController *PIDController_Create(float kp,float ki,float kd,float tau,\
						float limmin,float limmax,float limmint,float limmaxt,float time) 
{
	PIDController *pid = (PIDController *)malloc(sizeof(PIDController));
	if(pid == NULL) return NULL;
	memset(pid,0,sizeof(PIDController));

	/* Controller gains */
	pid->Kp = kp;
	pid->Ki = ki;
	pid->Kd = kd;
	/* Derivative low-pass filter time constant */
	pid->tau = tau;
	/* Output limits */
	pid->limMin = limmin;
	pid->limMax = limmax;
	/* Integrator limits */
	pid->limMinInt = limmint;
	pid->limMaxInt = limmaxt;
	/* Sample time (in seconds) */
	pid->T = time;
	/* Clear controller variables */
	pid->integrator = 0.0f;
	pid->prevError  = 0.0f;
	pid->differentiator  = 0.0f;
	pid->prevMeasurement = 0.0f;
	/* Controller output */
	pid->out = 0.0f;

	return pid;
}

void PIDController_Delete(PIDController *pid)
{
	if(pid){ free(pid); }
}

float PIDController_Update(PIDController *pid, float setpoint, float measurement,bool check_lim) 
{
	// Error signal
    float error = setpoint - measurement;
	// Proportional
    float proportional = pid->Kp * error;
	// Integral
    pid->integrator = pid->integrator + 0.5f * pid->Ki * pid->T * (error + pid->prevError);
	if(!check_lim)
		goto cal_pid;
	// Anti-wind-up via integrator clamping
    if (pid->integrator > pid->limMaxInt) 
	{
        pid->integrator = pid->limMaxInt;
    } 
	else if (pid->integrator < pid->limMinInt) 
	{
        pid->integrator = pid->limMinInt;
    }
cal_pid:	
	// Derivative (band-limited differentiator)
	//Note: derivative on measurement, therefore minus sign in front of equation!
    pid->differentiator = 	- (2.0f * pid->Kd * (measurement - pid->prevMeasurement)\
							+ (2.0f * pid->tau - pid->T) * pid->differentiator)		\
							/ (2.0f * pid->tau + pid->T);
	// Compute output and apply limits
    pid->out = proportional + pid->integrator + pid->differentiator;
	if(!check_lim)
		goto out_pid;
    if (pid->out > pid->limMax) 
	{
        pid->out = pid->limMax;
    }
	else if (pid->out < pid->limMin) 
	{
        pid->out = pid->limMin;
    }
out_pid:
	// Store error and measurement for later use
    pid->prevError       = error;
    pid->prevMeasurement = measurement;
	// Return controller output
    return pid->out;
}
