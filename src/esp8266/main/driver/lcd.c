//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	lcd.c												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "driver/i2c.h"
#include "lcd.h"
//---------------------------------------//

//---------------GLOBAL------------------//
//---------------------------------------//

//--------------PRIVATE------------------//
// static uint8_t    _addr = 0x27;
// static uint8_t    _cols = 16;
static uint8_t    _rows = 2;
// static uint8_t    _charsize;
static uint8_t    _backlightval = LCD_BACKLIGHT;
static uint8_t    _displayfunction;
static uint8_t    _displaycontrol;
static uint8_t    _displaymode;


static const char *TAG = "LCD";
//---------------------------------------//


static esp_err_t i2c_example_master_init()
{
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;
    conf.sda_pullup_en = 1;
    conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;
    conf.scl_pullup_en = 1;
    conf.clk_stretch_tick = 300; // 300 ticks, Clock stretch is about 210us, you can make changes according to the actual situation.
    ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &conf));
    return ESP_OK;
}



static esp_err_t expanderWrite(uint8_t data) //expander_write
{
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LCD_ADDR<<1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data|BACKLIGHT, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;

}

static esp_err_t pulseEnable(uint8_t data)
{
    int ret;
    ret  = expanderWrite(data | En);  // En high
    ets_delay_us(1);       // enable pulse must be >450ns
    ret |= expanderWrite(data & ~En); // En low
    ets_delay_us(50);      // commands need > 37us to settle
    return ret; 
}

static esp_err_t write4bits(uint8_t data) //write4bit_lcd
{//, 4bit high-> data, 4 bit low-> control
    int ret;
    ret  = expanderWrite(data);
    ret |= pulseEnable(data);
    return ret;
} 

static esp_err_t send (uint8_t data,uint8_t mode) //send data byte
{
    int ret;
    uint8_t highnib=data&0xf0;
    uint8_t lownib=(data<<4)&0xf0;
    ret  = write4bits((highnib)|mode);
    ret |= write4bits((lownib)|mode);
    return ret;
}

static esp_err_t command(uint8_t cmd) //command
{
    return send(cmd,LCD_CMD);
}

static esp_err_t write(uint8_t data) //data
{
    return send(data,LCD_DATA);
}



static void display() 
{
    _displaycontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

static void backlight(void) 
{
    _backlightval=LCD_BACKLIGHT;
    expanderWrite(0);
}

static esp_err_t lcd_begin(void)
{
    int ret;
    ret  = i2c_example_master_init();

    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;


    if (_rows > 1) {
        _displayfunction |= LCD_2LINE;
    }

    // // for some 1 line displays you can select a 10 pixel high font
    // if ((_charsize != 0) && (_rows == 1)) {
    //     _displayfunction |= LCD_5x10DOTS;
    // }

    vTaskDelay(50 / portTICK_RATE_MS);
    ret |= expanderWrite(_backlightval);
    vTaskDelay(1000 / portTICK_RATE_MS);

    write4bits(0x03 << 4);
    ets_delay_us(4500);

    write4bits(0x03 << 4);
    ets_delay_us(4500);
        
    write4bits(0x03 << 4);
    ets_delay_us(150);


    write4bits(0x02 << 4);

    // set # lines, font size, etc.
    command(LCD_FUNCTIONSET | _displayfunction);

    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();

    // clear it off
    clear();

    // Initialize to default text direction (for roman languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

    // set the entry mode
    command(LCD_ENTRYMODESET | _displaymode);
    home();
    return ret;
}



void init_lcd(void)
{
  	int ret = lcd_begin();
  	backlight();
  	setCursor(0,0);
    if(ret == ESP_FAIL)
    {
        ESP_LOGI(TAG,"Sending command error, I2C slave doesn't ACK the transfer.");
    }
    else
        ESP_LOGI(TAG,"I2C Init Success.");
}

void printstr(char* c)
{
    while(*(c))
    {    
        taskENTER_CRITICAL();
        write(*(c++)); 
        taskEXIT_CRITICAL();
    }
}

void setCursor(uint8_t col, uint8_t row)
{
    int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
    if (row > _rows) {
        row = _rows-1;    // we count rows starting w/0
    }
    command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void clear(void)
{
    command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
    ets_delay_us(2000);  // this command takes a long time!
}


void home(void)
{
    command(LCD_RETURNHOME);  // set cursor position to zero
    ets_delay_us(2000);  // this command takes a long time!
}