#ifndef _TCP_H_
#define _TCP_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	tcp.h												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
//---------------------------------------//

//---------------MACRO-------------------//
typedef enum
{
    TCP_REQUEST_GET = 0,
    TCP_REQUEST_POST,
}tcp_request_type_t;


//---------------------------------------//

//--------------DECLARE------------------//
void tcp_server_start(void );
void tcp_server_stop(void);
void tcp_data_handler_register(void (*callback)(char* data));
void tcp_server_push_notify(char *data);
void register_tcp_data_sent_cb(void (*callback)(void));
//---------------------------------------//
#endif//_TCP_H_