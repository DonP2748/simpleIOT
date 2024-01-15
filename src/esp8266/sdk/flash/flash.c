//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0		//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	flash.c																								//
// AUTHOR 	: 	DonP 																								//
// DATE   	: 	--/--/----																					//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "flash.h"
//---------------------------------------//

//---------------GLOBAL------------------//
#define TAG "FLASH"
//---------------------------------------//

//--------------PRIVATE------------------//

//---------------------------------------//

  

bool write_spi_flash_items (char* item, char* data)
{
	char* FileName = item;
	char* FilePath = (char*)calloc(strlen("/spiffs/") + strlen(FileName) + strlen(".txt"),sizeof(char));
	strcat(FilePath,"/spiffs/");
	strcat(FilePath,FileName);
	strcat(FilePath,".txt");

    FILE* lf = fopen(FilePath,"w");
    if (lf == NULL)
    {
  	     ESP_LOGE(TAG, "SPIFFS OPEN FAIL to write");
  	     return 0;
    } 
  	fprintf(lf,"%s\n",data);
	fclose(lf);
    free(FilePath);
	return 1;
}

bool read_spi_flash_items (char* item, char* data, uint32_t len)
{
    char* FileName = item;
    char* FilePath = (char*)calloc(strlen("/spiffs/") + strlen(FileName) + strlen(".txt"),sizeof(char));
    strcat(FilePath,"/spiffs/");
    strcat(FilePath,FileName);
    strcat(FilePath,".txt");

    FILE* lf = fopen(FilePath,"r");
    if(!lf) 
    {
        ESP_LOGE(TAG, "SPIFFS OPEN FAIL to read");
        return 0; 
    }
    fgets(data,len,lf);
    fclose(lf);
    free(FilePath);
	return 1;
}


bool erase_spi_flash_items (char* item)
{
    char* FileName = item;
    char* FilePath = (char*)calloc(strlen("/spiffs/") + strlen(FileName) + strlen(".txt"),sizeof(char));
    strcat(FilePath,"/spiffs/");
    strcat(FilePath,FileName);
    strcat(FilePath,".txt");
    struct stat st;
    if (stat(FilePath, &st) != 0) 
    {
        ESP_LOGE(TAG, "SPIFFS File item does not exist!");
        free(FilePath);
        return 0;
    }
    // Delete it if it exists
    unlink(FilePath);
    free(FilePath);
    return 1;
}



void spi_flash_mount()
{
    esp_vfs_spiffs_conf_t conf = 
    {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) 
    {
        if (ret == ESP_FAIL) 
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
}

void spi_flash_unmount() 
{
	int ret = esp_vfs_spiffs_unregister(NULL);
    if(ret != ESP_OK) 
        ESP_LOGI(TAG, "SPIFFS already unmounted");
}


