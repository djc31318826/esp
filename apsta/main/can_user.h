/* 
   WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "esp_netif.h"
#include "esp_netif_types.h"
#include "esp_http_server.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp32s3/spiram.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>


typedef enum{
   CHANGE_STATE ,
   STOP_RECIEVE ,
   STOP_SEND    ,
   START_RECIEVE,
   START_SEND   ,
   START_SEND_ONCE,
   STOP_CAN     ,
   START_CAN    ,
}message_type_t;

typedef struct _tag_parse_resp_arg
{
    uint8_t buf[16];
    int len;
}parse_resp_arg;

typedef struct _tagStructCanMessage
{
   /* data */
   message_type_t  message_type;
}StructCanMessage;

extern QueueHandle_t xCanQueueRoutine;
extern QueueHandle_t xCanQueueRecieve;
extern QueueHandle_t xCanQueueSend;

extern SemaphoreHandle_t RecieveMutex;
extern SemaphoreHandle_t SendMutex   ;

extern twai_general_config_t g_config;
extern twai_timing_config_t t_config;
extern twai_filter_config_t f_config;

#define TX_GPIO_NUM                     CONFIG_EXAMPLE_TX_GPIO_NUM
#define RX_GPIO_NUM                     CONFIG_EXAMPLE_RX_GPIO_NUM

extern twai_general_config_t g_config ;
extern twai_timing_config_t t_config  ;//TWAI_TIMING_CONFIG_25KBITS();
extern twai_filter_config_t f_config  ;

extern twai_message_t tx_can_message;
extern twai_message_t rx_can_message;

void init_can(int can_brp);
void close_can();

void twai_receive_task(void *arg);
void twai_send_task(void *arg);
//void twai_ctrl_task(void *arg);
void twai_routine(void *arg);
extern void parse_can_message(uint8_t *arg,int len);

void wss_server_can_send_messages(httpd_handle_t *server,void *arg,int len);//send_mode 0:发送rgb值，1:发送CAN的消息；
void ws_async_send_can(void *arg);
