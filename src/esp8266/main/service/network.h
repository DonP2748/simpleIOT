#ifndef _NETWORK_H_
#define _NETWORK_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	network.h											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
//---------------------------------------//

//---------------MACRO-------------------//
//---------------------------------------//

//--------------DECLARE------------------//
void init_network_process(void);
void get_network_info(int* rssi,uint8_t* mac,char* ssid, uint32_t* ip);
//---------------------------------------//
#endif//_NETWORK_H_