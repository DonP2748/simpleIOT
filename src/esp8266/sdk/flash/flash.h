#ifndef _FLASH_H
#define _FLASH_H

//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	flash.h												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
//---------------------------------------//

//---------------MACRO-------------------//
//---------------------------------------//

//--------------DECLARE------------------//
bool read_spi_flash_items (char* item, char* data, uint32_t len);
bool write_spi_flash_items (char* item, char* data);
bool erase_spi_flash_items (char* item);
void spi_flash_mount(void); 
void spi_flash_unmount(void); 

//---------------------------------------//
#endif//_FLASH_H