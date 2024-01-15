#ifndef _JSON_PARSE_H_
#define _JSON_PARSE_H_

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	jsonparse.h											//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
//---------------------------------------//

//---------------MACRO-------------------//
//---------------------------------------//

//--------------DECLARE------------------//
void init_json_process(void);
void create_json_msg(void);
void relese_json_msg(void);
char* get_json_msg(void);
void create_object_json (uint8_t evt, void* dev);
//void register_create_msg_json_cb(void(*callback)(void* data));
void register_process_data_json_cb(void(*callback)(void* data));
//---------------------------------------//
#endif//_JSON_PARSE_H_