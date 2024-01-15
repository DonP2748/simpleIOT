//////////////////////////////////////////////////////////////////////
// All rights reserved. This program and the accompanying materials	//
// are made available under the terms of the Public License v1.0	//
// and Distribution License v1.0 which accompany this distribution.	//
// MODULE  	:	tcp.c												//
// AUTHOR 	: 	DonP 												//
// DATE   	: 	--/--/----											//
//////////////////////////////////////////////////////////////////////


//--------------INCLUDE------------------//
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "lwip/sockets.h"
#include "tcp.h"
//---------------------------------------//

//---------------GLOBAL------------------//
#define PORT 3333
const int IPV4_GOTIP_BIT = BIT0;
int gb_sock = 0;    //global

//---------------------------------------//

//--------------PRIVATE------------------//
static const char *TAG = "TCP_SERVER";
static void (*tcp_server_request_handler)(char* data) = NULL;
static void tcp_server_task(void *pvParameters);
//---------------------------------------//



void tcp_server_push_notify(char *data)
{
    if(gb_sock)
    {
        int err = send(gb_sock, data, strlen(data), 0);
        if (err < 0) 
        {
            ESP_LOGE(TAG, "Error occured during sending: errno %d", errno);
        }
    }
}

void tcp_server_start(void)
{
    ESP_LOGI(TAG, "start tcp server");
    xTaskCreate(tcp_server_task, "tcp_server", 4096, NULL, 5, NULL);
}


void tcp_server_stop(void)
{
    ESP_LOGI(TAG, "stop tcp server");
    shutdown(gb_sock, 0);
    close(gb_sock);
    return;
}



void tcp_data_handler_register(void (*callback)(char* data))
{
    if(callback)
        tcp_server_request_handler = callback;
}

static void tcp_server_task(void *pvParameters)
{
    char rx_buffer[1024];
    char addr_str[128];
    int addr_family;
    int ip_protocol;
     
    while (1) 
    {
        struct sockaddr_in destAddr;
        destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        destAddr.sin_family = AF_INET;
        destAddr.sin_port = htons(PORT);
        addr_family = AF_INET;
        ip_protocol = IPPROTO_IP;
        inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);
        int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (listen_sock < 0) 
        {
            ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket created");

        int err = bind(listen_sock, (struct sockaddr *)&destAddr, sizeof(destAddr));
        if (err != 0) 
        {
            ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket binded");

        err = listen(listen_sock, 1);
        if (err != 0) 
        {
            ESP_LOGE(TAG, "Error occured during listen: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_in sourceAddr;

        

        uint addrLen = sizeof(sourceAddr);
        int sock = accept(listen_sock, (struct sockaddr *)&sourceAddr, &addrLen);

        

        gb_sock = sock;
        if (sock < 0) 
        {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Socket accepted");

        while (1) 
        {
            int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            // Error occured during receiving
            if (len < 0) 
            {
                ESP_LOGE(TAG, "recv failed: errno %d", errno);
                break;
            }
            // Connection closed
            else if (len == 0) 
            {
                ESP_LOGI(TAG, "Connection closed");
                break;
            }
            // Data received
            else {

                inet_ntoa_r(((struct sockaddr_in *)&sourceAddr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);

                rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string

                if(tcp_server_request_handler)
                    tcp_server_request_handler(rx_buffer);
            }
        }

        if (sock != -1) 
        {
            ESP_LOGE(TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0);
            close(sock);
        }
    }
    vTaskDelete(NULL);
}