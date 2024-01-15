#ifndef _AES_H
#define _AES_H

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	aes.h												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
//---------------------------------------//

//---------------MACRO-------------------//
#define AES_KEY_LENGTH		16
#define HMAC_KEY_LENGTH		64
#define SYS_FLASH_SIGN	0x6DB69249

typedef struct sys_flash_t {
	uint32_t sign;
	uint8_t aes_key[AES_KEY_LENGTH];
	uint8_t mac_key[HMAC_KEY_LENGTH];
} sys_flash_t;


extern uint8_t aes_key[];
extern uint8_t mac_key[];
extern bool key_loaded;

//---------------------------------------//

//--------------DECLARE------------------//
void aes_encrypt_cfb128(const uint8_t* input, int input_size, uint8_t* output, const uint8_t* aes_key); 
bool aes_decrypt_verify(const uint8_t* input, int input_size, uint8_t* output, const uint8_t* aes_key); 
bool aes_load_keys(void); 
bool aes_save_keys(void); 

//---------------------------------------//
#endif//_AES_H