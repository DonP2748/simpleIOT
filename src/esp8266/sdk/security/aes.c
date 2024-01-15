//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	aes.c												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "esp_log.h"
#include "mbedtls/aes.h"
#include "flash/flash.h"
#include "aes.h"
//---------------------------------------//

//---------------GLOBAL------------------//
#define MAC_LENGTH			20
uint8_t aes_key[AES_KEY_LENGTH] = { 0 };
uint8_t mac_key[HMAC_KEY_LENGTH] = { 0 };
bool key_loaded = false;
//---------------------------------------//

//--------------PRIVATE------------------//
static mbedtls_aes_context context;
static uint8_t enc_iv[AES_KEY_LENGTH] = { 0 };
static size_t enc_iv_offset = 0;
static uint8_t dec_iv[AES_KEY_LENGTH] = { 0 };
static size_t dec_iv_offset = 0;
// static void append_padding(uint8_t* buffer, int input_length, int full_length);
// static bool remove_padding(uint8_t* buffer, int full_length, int* output_length);
//---------------------------------------//

void aes_encrypt_cfb128(const uint8_t* input, int input_size, uint8_t* output, const uint8_t* aes_key) 
{
    mbedtls_aes_init(&context);
    mbedtls_aes_setkey_enc(&context, aes_key, 128);
    mbedtls_aes_crypt_cfb128(&context, MBEDTLS_AES_ENCRYPT, input_size, &enc_iv_offset, enc_iv, input, output);
	mbedtls_aes_free(&context);
}

bool aes_decrypt_verify(const uint8_t* input, int input_size, uint8_t* output, const uint8_t* aes_key) 
{
	mbedtls_aes_init(&context);
    // mbedtls_aes_setkey_dec(&context, aes_key, 128); //Do CFB, chi dung enc or dec cho ca 2 action
    mbedtls_aes_setkey_enc(&context, aes_key, 128);
	mbedtls_aes_crypt_cfb128(&context, MBEDTLS_AES_DECRYPT, input_size, &dec_iv_offset, dec_iv, input, output);
    mbedtls_aes_free(&context);
	return true;
}

bool aes_load_keys(void) 
{
	bool succeeded = true;
	sys_flash_t* sf = (sys_flash_t*)malloc(sizeof(sys_flash_t));
	if (sf == NULL) 
	{
		goto error;
	}
	spi_flash_mount();
	if (!read_spi_flash_items("security",(char*)sf,sizeof(sys_flash_t))) 
	{
		goto error;
	}

	if (sf->sign != SYS_FLASH_SIGN) 
	{
		goto error;
	}
	memcpy(aes_key, sf->aes_key, AES_KEY_LENGTH);	
	memcpy(mac_key, sf->mac_key, HMAC_KEY_LENGTH);	
	key_loaded = true;
	goto done;	
error:
	succeeded = false;
done:
	if (sf != NULL) {
		free(sf);
	}
	spi_flash_unmount();
	return succeeded;
}

bool aes_save_keys(void) 
{
	bool succeeded = true;
	sys_flash_t* sf = (sys_flash_t*)malloc(sizeof(sys_flash_t));	
	if (sf == NULL) 
	{
		goto error;
	}

	sf->sign = SYS_FLASH_SIGN;	
	memcpy(sf->aes_key, aes_key, AES_KEY_LENGTH);	
	memcpy(sf->mac_key, mac_key, HMAC_KEY_LENGTH);
	spi_flash_mount();
	if (!write_spi_flash_items("security",(char*)sf)) 
	{	
		goto error;
	}
	goto done;

error:
	succeeded = false;
done:
	if (sf != NULL) 
	{	
		free(sf);
	}
	spi_flash_unmount();
	return succeeded;
}
// Use for CBC
// static void append_padding(uint8_t* buffer, int input_length, int full_length) 
// {
// 	int index;
// 	uint8_t pad = full_length - input_length;
// 	for (index = input_length; index < full_length; index++)
// 		buffer[index] = pad;	
// }

// static bool remove_padding(uint8_t* buffer, int full_length, int* output_length) 
// {
// 	uint8_t pad = buffer[full_length - 1];
// 	int length = full_length - pad;	
// 	int index;
// 	if (full_length < pad)
// 		return false;
// 	for (index = length; index < full_length - 1; index++) 
// 	{	
// 		if (buffer[index] != pad)
// 			return false;
// 	}
// 	if (output_length != NULL)
// 		*output_length = length;	
// 	return true;
// }