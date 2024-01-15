//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials //
// are made available under the terms of the Public License v1.0    //
// and Distribution License v1.0 which accompany this distribution. //
// MODULE   :   sensor.c                                            //
// AUTHOR   :   DonP                                                //
// DATE     :   --/--/----                                          //
//////////////////////////////////////////////////////////////////////

//--------------INCLUDE------------------//
#include <stdlib.h>
#include <sys/unistd.h>
#include <stdio.h>
#include <string.h>
#include <sensor.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include <driver/gpio.h>
#include "esp_log.h"
#include <rom/ets_sys.h> // os_delay_us
//---------------------------------------//

//---------------GLOBAL------------------//
// DHT timer precision in microseconds
#define DHT_TIMER_INTERVAL  2
#define DHT_DATA_BITS       40
#define DHT_PIN             4
typedef enum
{
    DHT_TYPE_DHT11 = 0, //!< DHT11
    DHT_TYPE_DHT22,     //!< DHT22
    DHT_TYPE_SI7021     //!< Itead SI7021
} dht_sensor_type_t;

//---------------------------------------//

//--------------PRIVATE------------------//
static const char *TAG = "SENSOR";
static bool dht_await_pin_state(uint8_t pin, uint32_t timeout, bool expected_pin_state, uint32_t *duration);

static esp_err_t dht_init(gpio_num_t pin, bool pull_up); 
static esp_err_t dht_read_data(dht_sensor_type_t sensor_type, gpio_num_t pin, int16_t *humidity, int16_t *temperature);
static esp_err_t dht_read_float_data(dht_sensor_type_t sensor_type, gpio_num_t pin, float *humidity, float *temperature);

//---------------------------------------//


void sensor_init(void)
{
    dht_init(DHT_PIN,1);
}

esp_err_t sensor_read_data(sensor_t* data)
{
    int16_t temp,humi;
    //ESP8266 get trouble when prinf, sprintf and ESP_LOGI with float number
    int ret = dht_read_data(DHT_TYPE_DHT11,DHT_PIN,&humi,&temp);
    data->humi = (int)humi/10;
    data->temp = (int)temp/10;
    return ret;
}

esp_err_t sensor_read_float_data(float *t, float *h)
{
    return dht_read_float_data(DHT_TYPE_DHT11,DHT_PIN,h,t);
}


static esp_err_t dht_init(gpio_num_t pin, bool pull_up) 
{
    gpio_config_t io_conf = 
    {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT_OD,
        .pin_bit_mask = 1ULL << pin,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = pull_up ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE
    };
    esp_err_t result = gpio_config(&io_conf);
    if (result == ESP_OK) 
    {
        gpio_set_level(pin, 1);
        return ESP_OK;
    }
    return result;
}

static bool dht_await_pin_state(uint8_t pin, uint32_t timeout,
        bool expected_pin_state, uint32_t *duration)
{
    for (uint32_t i = 0; i < timeout; i += DHT_TIMER_INTERVAL) {
        // need to wait at least a single interval to prevent reading a jitter
        os_delay_us(DHT_TIMER_INTERVAL);
        if (gpio_get_level(pin) == expected_pin_state) {
            if (duration) {
                *duration = i;
            }
            return true;
        }
    }

    return false;
}


static inline bool dht_fetch_data(dht_sensor_type_t sensor_type, uint8_t pin, bool bits[DHT_DATA_BITS])
{
    uint32_t low_duration;
    uint32_t high_duration;

    // Phase 'A' pulling signal low to initiate read sequence
    gpio_set_level(pin, 0);
    os_delay_us(sensor_type == DHT_TYPE_SI7021 ? 500 : 20000);
    gpio_set_level(pin, 1);

    // Step through Phase 'B', 40us
    if (!dht_await_pin_state(pin, 40, false, NULL))
    {
        //ESP_LOGI(TAG,"Initialization error, problem in phase 'B'\n");
        return false;
    }

    // Step through Phase 'C', 88us
    if (!dht_await_pin_state(pin, 88, true, NULL)) 
    {
        //ESP_LOGI(TAG,"Initialization error, problem in phase 'C'\n");
        return false;
    }

    // Step through Phase 'D', 88us
    if (!dht_await_pin_state(pin, 88, false, NULL)) 
    {
        //ESP_LOGI(TAG,"Initialization error, problem in phase 'D'\n");
        return false;
    }

    // Read in each of the 40 bits of data...
    for (int i = 0; i < DHT_DATA_BITS; i++) {
        if (!dht_await_pin_state(pin, 65, true, &low_duration)) 
        {
            //ESP_LOGI(TAG,"LOW bit timeout\n");
            return false;
        }
        if (!dht_await_pin_state(pin, 75, false, &high_duration)) 
        {
            //ESP_LOGI(TAG,"HIGHT bit timeout\n");
            return false;
        }
        bits[i] = high_duration > low_duration;
    }
    return true;
}


static inline int16_t dht_convert_data(dht_sensor_type_t sensor_type, uint8_t msb, uint8_t lsb)
{
    int16_t data;

    if (sensor_type == DHT_TYPE_DHT22) {
        data = msb & 0x7F;
        data <<= 8;
        data |= lsb;
        if (msb & BIT(7)) {
            data = 0 - data;       // convert it to negative
        }
    }
    else {
        data = msb * 10;
    }

    return data;
}


static esp_err_t dht_read_data(dht_sensor_type_t sensor_type, gpio_num_t pin, int16_t *humidity, int16_t *temperature)
{
    bool bits[DHT_DATA_BITS];
    uint8_t data[DHT_DATA_BITS/8] = {0};
    bool result;

    taskENTER_CRITICAL();
    result = dht_fetch_data(sensor_type, pin, bits);
    taskEXIT_CRITICAL();

    if (!result) 
    {
        return ESP_FAIL;
    }

    for (uint8_t i = 0; i < DHT_DATA_BITS; i++) 
    {
        // Read each bit into 'result' byte array...
        data[i/8] <<= 1;
        data[i/8] |= bits[i];
    }

    if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) 
    {
        //ESP_LOGI(TAG,"Checksum failed, invalid data received from sensor\n");
        return ESP_FAIL;
    }

    *humidity = dht_convert_data(sensor_type, data[0], data[1]);
    *temperature = dht_convert_data(sensor_type, data[2], data[3]);

    ESP_LOGI(TAG,"Sensor data: humidity=%d, temp=%d\n", *humidity, *temperature);

    return ESP_OK;
}

static esp_err_t dht_read_float_data(dht_sensor_type_t sensor_type, gpio_num_t pin, float *humidity, float *temperature)
{
    int16_t i_humidity, i_temp;

    if (dht_read_data(sensor_type, pin, &i_humidity, &i_temp) == ESP_OK) 
    {
        *humidity = (float)i_humidity / 10; //failed o day ?:D? rat kho hieu, print ra empty
        *temperature = (float)i_temp / 10;
        ESP_LOGI(TAG,"Sensor data: humidity=%.2f, temp=%.2f\n",*humidity,*temperature); //in ra empty ca 2 t va h
        return ESP_OK;
    }
    return ESP_FAIL;
}

