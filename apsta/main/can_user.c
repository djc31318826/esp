#include "can_user.h"
#include "global_var.h"
QueueHandle_t xCanQueue=NULL;
QueueHandle_t xCanQueueRoutine=NULL;
QueueHandle_t xCanQueueRecieve=NULL;
QueueHandle_t xCanQueueSend=NULL;
SemaphoreHandle_t RecieveMutex=NULL;
SemaphoreHandle_t SendMutex   =NULL;

twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(TX_GPIO_NUM, RX_GPIO_NUM, TWAI_MODE_NORMAL);
twai_timing_config_t t_config  = TWAI_TIMING_CONFIG_1MBITS();
twai_filter_config_t f_config  = TWAI_FILTER_CONFIG_ACCEPT_ALL();

twai_message_t tx_can_message={.identifier = 2, .data_length_code = 8,
                                     .data = {0xaa, 0xff , 0 , 0 ,0 ,0 ,0 ,0}};
twai_message_t rx_can_message={.identifier = 2, .data_length_code = 8,
                                     .data = {0xff, 0xaa , 0 , 0 ,0 ,0 ,0 ,0}};
long rcv_cnt=0;
long send_cnt=0;
//#define DEBUG
void init_can(int can_brp)
{


}

void close_can()
{


}

//void twai_task(void *arg)
//{
    //获取信号量开始任务的分发，INSTALL_CAN_DRIVER,UNINSTALL_CAN_DRIVER,SEND_MESSAGE,REC_MESSAGE,EXIT_TASK


//}

void twai_receive_task(void *arg)
{
    //StructCanMessage xSend;
    StructCanMessage TokenQuence;
    esp_err_t ret;
    while(1)
    {
        //等待接收启动的信号
        while(1)
        {
            if(xCanQueueRecieve!=NULL)
            {
                if(xQueueReceive(xCanQueueRecieve,&TokenQuence,/*0*/pdMS_TO_TICKS(10))==pdPASS)
                {
                    if(TokenQuence.message_type==START_RECIEVE)
                    {
                        //xSemaphoreGive(SendMutex)
                        printf("can wating for rec message\n");
                        break;
                    }
                }
            }
        }
        while(1)
        {
            if(xCanQueueRecieve!=NULL)
            {
                if(xQueueReceive(xCanQueueRecieve,&TokenQuence,/*0*/pdMS_TO_TICKS(10))==pdPASS)
                {
                    if(TokenQuence.message_type==STOP_RECIEVE)
                    {
                        printf("recieve mutex give\n");
                        xSemaphoreGive(RecieveMutex);
                        break;
                    }
                }
            }    
            
            //接收的代码处理
            //printf("Recieved message0\n");
            //twai_message_t rx_can_message_t;
            //ret=twai_receive(&rx_can_message,pdMS_TO_TICKS(500));
            rx_can_message.data_length_code=0;
            ret=twai_receive(&rx_can_message,pdMS_TO_TICKS(10));
            //printf("Recieved message1\n");
            //将接收到的消息发送到网络
            //2022-08-13需要循环读取，每次只读一包的话，很可能会导致报文累积
            if(ret==ESP_OK)
            {
            //    typedef struct {
            //         union {
            //            struct {
            //            //The order of these bits must match deprecated message flags for compatibility reasons
            //            uint32_t extd: 1;           /**< Extended Frame Format (29bit ID) */
            //            uint32_t rtr: 1;            /**< Message is a Remote Frame */
            //            uint32_t ss: 1;             /**< Transmit as a Single Shot Transmission. Unused for received. */
            //            uint32_t self: 1;           /**< Transmit as a Self Reception Request. Unused for received. */
            //            uint32_t dlc_non_comp: 1;   /**< Message's Data length code is larger than 8. This will break compliance with ISO 11898-1 */
            //            uint32_t reserved: 27;      /**< Reserved bits */
            //        };
            //        //Todo: Deprecate flags
            //        uint32_t flags;                 /**< Deprecated: Alternate way to set bits using message flags */
            //};
            //        uint32_t identifier;                /**< 11 or 29 bit identifier */
            //        uint8_t data_length_code;           /**< Data length code */
            //        uint8_t data[TWAI_FRAME_MAX_DLC];    /**< Data bytes (not relevant in RTR frame) */
            //} twai_message_t;
                //extd,rtr,identifier(4 bytes),data[]--根据data_length_code
                rcv_cnt++;
//#ifdef DEBUG                
                printf("rcv_cnt=%ld,Recieved message %2.2x-%2.2x-0x%4.4x\n",rcv_cnt,rx_can_message.extd,rx_can_message.rtr,rx_can_message.identifier);
//#endif                
                int  len=rx_can_message.data_length_code+8;
                void *arg = malloc(len);
                *((unsigned char *)arg+0)=0;
                *((unsigned char *)arg+1)=1;
                *((unsigned char *)arg+2)=rx_can_message.extd;
                *((unsigned char *)arg+3)=rx_can_message.rtr;
                *((unsigned char *)arg+4)=(rx_can_message.identifier&0xff);
                *((unsigned char *)arg+5)=((rx_can_message.identifier>>8)&0xff);
                *((unsigned char *)arg+6)=((rx_can_message.identifier>>16)&0xff);
                *((unsigned char *)arg+7)=((rx_can_message.identifier>>24)&0xff);
                
                for(int i=0;i<rx_can_message.data_length_code;i=i+1)
                {
                    *((unsigned char *)arg+8+i)=rx_can_message.data[i];
//#ifdef DEBUG                       
                    printf("rx_can_message.data[%d]=%2.2x\n",i,rx_can_message.data[i]);
//#endif
                }
                
                twai_status_info_t status_info;
#if 1    
                if(twai_get_status_info(&status_info)==ESP_OK)
                {
                   
                    printf("c twai_get_status_info(msg_tx,tx_failed_cnt,tx_err_cnt,bus_err_cnt,state,rx_missed_count,rx_overrun_count,msgs_to_rx) %d,%d,%d,%d,%d,%d,%d,%d\n",
             
                    status_info.msgs_to_tx,status_info.tx_failed_count,status_info.tx_error_counter,status_info.bus_error_count,status_info.state,status_info.rx_missed_count,status_info.rx_overrun_count,status_info.msgs_to_rx);
                    ESP_LOGI("wifi", "free Heap:%d,%d", esp_get_free_heap_size(), heap_caps_get_free_size(MALLOC_CAP_8BIT));
                }
                printf("\n");
#endif  
                //printf("arg=%p\n",arg);
                wss_server_can_send_messages(&handle,arg,len);
                //free(arg);
                //printf("handle=%p\n",handle);
                //ret=twai_receive(&rx_can_message,pdMS_TO_TICKS(1));
            }
        }
    }
}

struct async_can_arg
{
    httpd_handle_t hd;
    int fd;
    void *ptr;
    int len;
};

void ws_async_send_can(void *arg)
{
    // static const char * data = "Async data";
    //
    // static int i=0;
   
    struct async_can_arg *resp_arg = arg;
    httpd_handle_t hd = resp_arg->hd;
    int fd = resp_arg->fd;
    httpd_ws_frame_t ws_pkt;
#ifdef DEBUG        
    printf("ws_async_send_can\n");
#endif    
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.payload = (uint8_t *)resp_arg->ptr; //(uint8_t*)data;
    ws_pkt.len = resp_arg->len;        // strlen(data);
    ws_pkt.type = HTTPD_WS_TYPE_BINARY;

    httpd_ws_send_frame_async(hd, fd, &ws_pkt);
    free(resp_arg->ptr);
    free(resp_arg);
#ifdef DEBUG    
    printf("ws_async_send_can_end\n");
#endif
}


void wss_server_can_send_messages(httpd_handle_t *server,void * arg, int len)//send_mode 0:发送rgb值，1:发送CAN的消息；
{
    bool send_messages = true;

    // Send async message to all connected clients that use websocket protocol every 10 seconds
    // while (send_messages) {
    // vTaskDelay(10000 / portTICK_PERIOD_MS);

    if (!*server)
    { // httpd might not have been created by now
        // continue;
        return;
    }
    size_t clients = MAX_CLIENTS;
    int client_fds[MAX_CLIENTS] = {0};
#ifdef DEBUG        
    printf("wss_server_can_send_messages\n");
#endif
    if (httpd_get_client_list(*server, &clients, client_fds) == ESP_OK)
    {
        //printf("0000\n");
        for (size_t i = 0; i < clients; ++i)
        {
#ifdef DEBUG                
            printf("0-i=%d\n",i);
#endif            
            int sock = client_fds[i];
            
            if (httpd_ws_get_fd_info(*server, sock) == HTTPD_WS_CLIENT_WEBSOCKET)
            {
                //ESP_LOGI(TAG, "Active client (fd=%d) -> sending async message", sock);
#ifdef DEBUG                    
                printf("1-i=%d\n",i);
#endif
                struct async_can_arg *resp_arg = malloc(sizeof(struct async_can_arg));
                resp_arg->hd = *server;
                resp_arg->fd = sock;
                resp_arg->ptr = malloc(len);
                memcpy((void *)resp_arg->ptr,(void *)arg,len);
                resp_arg->len = len;
#ifdef DEBUG                    
                printf("arg1=%p\n",resp_arg->ptr);
#endif
                //free(arg);
                httpd_queue_work(handle, /*ws_async_send*/ws_async_send_can, resp_arg);
            }
        }
        free(arg);
    }
    else
    {
        //ESP_LOGE(TAG, "httpd_get_client_list failed!");
#ifdef DEBUG            
        printf("return\n");
#endif        
        return;
    }
    //}
}

void twai_send_task(void *arg)
{
    StructCanMessage TokenQuence;
    while(1)
    {
        //等待启动发送的信号
        while(1)
        {
            //vTaskDelay(pdMS_TO_TICKS(1));
            if(xCanQueueSend!=NULL)
            {
                if(xQueueReceive(xCanQueueSend,&TokenQuence,pdMS_TO_TICKS(10)/*0*/)==pdPASS)
                {
                    if(TokenQuence.message_type==START_SEND)
                    {
                        //xSemaphoreGive(SendMutex)
                        printf("can wating for send message\n");
                        break;
                    }
                }
            }
        }
        while(1)
        {
            if(xCanQueueSend!=NULL)
            {
                //printf("send running\n");
                if(xQueueReceive(xCanQueueSend,&TokenQuence,pdMS_TO_TICKS(500)/*0*/)==pdPASS)
                {
                    
                    if(TokenQuence.message_type==STOP_SEND)
                    {
                        printf("send mutex give\n");
                        xSemaphoreGive(SendMutex);
                        printf("send mutex gived\n");
                        
                        break;
                    }
                    //发送的代码处理
                    if(TokenQuence.message_type==START_SEND_ONCE)
                    {
//#ifdef DEBUG                            
                        printf("Can send message\n");

                        twai_status_info_t status_info;

                        if(twai_get_status_info(&status_info)==ESP_OK)
                        {
                            printf("b twai_get_status_info(msg_tx,tx_failed_cnt,tx_err_cnt,bus_err_cnt,state) %d,%d,%d,%d,%d\n",status_info.msgs_to_tx,status_info.tx_failed_count,status_info.tx_error_counter,status_info.bus_error_count,status_info.state);
                        }
//#endif
                        esp_err_t ret=twai_transmit(&tx_can_message,portMAX_DELAY);
//#ifdef DEBUG                        
                        if(ret!=ESP_OK)
                        {
                            printf("CAN Send Failed,ret=%d\n",ret);
                        }
                        else{
                            send_cnt++;
                        }
                    
                        if(twai_get_status_info(&status_info)==ESP_OK)
                        {
                            printf("a twai_get_status_info(msg_tx,tx_failed_cnt,tx_err_cnt,bus_err_cnt,state) %d,%d,%d,%d,%d\n",status_info.msgs_to_tx,status_info.tx_failed_count,status_info.tx_error_counter,status_info.bus_error_count,status_info.state);
                        }
                        printf("send_cnt=%ld,Send Once Done\n",send_cnt);
//#endif
                    }
                
                }
            }
        }
    }
}

void twai_routine(void *arg)
{
    //void *pMessage=malloc(sizeof(StructCanMessage));
    StructCanMessage msg;
    RecieveMutex=xSemaphoreCreateBinary();
    SendMutex   =xSemaphoreCreateBinary();
    if(xCanQueue==NULL)
    {
        xCanQueue=xQueueCreate(10,sizeof(StructCanMessage));
    }
    if(xCanQueueRecieve==NULL)
    {
        xCanQueueRecieve=xQueueCreate(10,sizeof(StructCanMessage));
    }

    if(xCanQueueSend==NULL)
    {
        xCanQueueSend=xQueueCreate(10,sizeof(StructCanMessage));
    }
    //while(1){vTaskDelay(pdMS_TO_TICKS(100));}
    
    printf("twai_routine runing\n");
    while(1)
    {
        //vTaskDelay(pdMS_TO_TICKS(100));
        //printf("twai_routine out\n");
        if(xQueueReceive(xCanQueue,&msg,pdMS_TO_TICKS(500)/*0*/)==pdPASS)
        {
            printf("Recieved Queue\n");
            printf("0 Recieved Queue=%d\n",msg.message_type);
            
            //开始解析pMessage的命令，然后做不同的处理。
            //命令1：重新初始化CAN和停止CAN时，首先需要暂停接收和发送任务。
            if(msg.message_type==CHANGE_STATE||msg.message_type==STOP_CAN||msg.message_type==START_CAN)
            {
                StructCanMessage xSend;
                StructCanMessage xRecieve;
                xSend.message_type   =STOP_SEND;
                xRecieve.message_type=STOP_RECIEVE;
                rcv_cnt=0;
                send_cnt=0;
                printf("Recieved Queue=%d\n",msg.message_type);
                //continue;
                //如果CAN已经启动了，需要暂停和卸载驱动
                twai_status_info_t status_info;
                if(twai_get_status_info(&status_info)==ESP_OK)
                {
                    printf("stoping\n");
                    if(status_info.state==TWAI_STATE_RUNNING)
                    {
                        //
                        printf("stop send and recieve\n");
                        xQueueSend(xCanQueueSend,&xSend,portMAX_DELAY);
                        printf("waiting send mutex\n");
                        xSemaphoreTake(SendMutex   ,portMAX_DELAY);//等待发送暂停                        
                        printf("waited send mutex\n");
                        printf("waiting rec mutex\n");
                        xQueueSend(xCanQueueRecieve,&xRecieve,portMAX_DELAY);
                        xSemaphoreTake(RecieveMutex,portMAX_DELAY);//等待接收暂停
                        printf("waited rec mutex\n");
                        //vTaskDelay(pdMS_TO_TICKS(200));
                        esp_err_t ret=twai_stop();
                        printf("twai_stop ret=%d\n",ret);
                        ret=twai_driver_uninstall();
                        printf("twai_driver_uninstall ret=%d\n",ret);
                        //continue;
                        //goto L1;
                    }
                    //vTaskDelay(pdMS_TO_TICKS(200));
                }
                printf("begin to install\n");
                if(msg.message_type!=STOP_CAN)
                {
                    //
                    printf("install can driver\n");
                    //以新的配置重新启动CAN总线
                    //g_config.rx_queue_len=256;
                    esp_err_t ret=twai_driver_install(&g_config,&t_config,&f_config);
                    printf("driver install ret=%d\n",ret);
                    ret=twai_start();
                    printf("twai start ret=%d\n",ret);
                    //继续接收任务和发送任务
                    xSend.message_type   =START_SEND;
                    xRecieve.message_type=START_RECIEVE;
                    xQueueSend(xCanQueueSend,&xSend,portMAX_DELAY);
                    xQueueSend(xCanQueueRecieve,&xRecieve,portMAX_DELAY);
                }
            }
        }
    }
    //free(pMessage);
}

void parse_can_message(uint8_t *arg,int len)
{
    //命令1：设置帧格式，数据格式和波特率 00-波特率-帧格式-数据格式 共4个byte
    //命令2：设置帧格式，数据格式和波特率 01-根据数据的长度决定的
    if(len==0)
    {
        printf("len=%d\n",len);
        return ;
    }
    StructCanMessage xSend;
    if(arg[0]==0&&arg[1]==0)
    {
        //设置命令
/*
#define TWAI_TIMING_CONFIG_25KBITS()    {.brp = 128, .tseg_1 = 16, .tseg_2 = 8, .sjw = 3, .triple_sampling = false}
#define TWAI_TIMING_CONFIG_50KBITS()    {.brp = 80, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}
#define TWAI_TIMING_CONFIG_100KBITS()   {.brp = 40, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}
#define TWAI_TIMING_CONFIG_125KBITS()   {.brp = 32, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}
#define TWAI_TIMING_CONFIG_250KBITS()   {.brp = 16, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}
#define TWAI_TIMING_CONFIG_500KBITS()   {.brp = 8, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}
#define TWAI_TIMING_CONFIG_800KBITS()   {.brp = 4, .tseg_1 = 16, .tseg_2 = 8, .sjw = 3, .triple_sampling = false}
#define TWAI_TIMING_CONFIG_1MBITS()     {.brp = 4, .tseg_1 = 15, .tseg_2 = 4, .sjw = 3, .triple_sampling = false}
*/
        if(arg[2]==0)
        {
            t_config.brp           =128;
            t_config.tseg_1        =16;
            t_config.tseg_2        =8;
            t_config.sjw           =3;
            t_config.triple_sampling=false;

        }
        if(arg[2]==1)
        {
            t_config.brp           =80;
            t_config.tseg_1        =15;
            t_config.tseg_2        =4;
            t_config.sjw           =3;
            t_config.triple_sampling=false;

        }
        if(arg[2]==2)
        {
            t_config.brp           =40;
            t_config.tseg_1        =15;
            t_config.tseg_2        =4;
            t_config.sjw           =3;
            t_config.triple_sampling=false;

        }
        if(arg[2]==3)
        {
            t_config.brp           =32;
            t_config.tseg_1        =15;
            t_config.tseg_2        =4;
            t_config.sjw           =3;
            t_config.triple_sampling=false;

        }
        if(arg[2]==4)
        {
            t_config.brp           =16;
            t_config.tseg_1        =15;
            t_config.tseg_2        =4;
            t_config.sjw           =3;
            t_config.triple_sampling=false;

        }
        if(arg[2]==5)
        {
            t_config.brp           =8;
            t_config.tseg_1        =15;
            t_config.tseg_2        =4;
            t_config.sjw           =3;
            t_config.triple_sampling=false;

        }
        if(arg[2]==6)
        {
            t_config.brp           =4;
            t_config.tseg_1        =16;
            t_config.tseg_2        =8;
            t_config.sjw           =3;
            t_config.triple_sampling=false;

        }
        if(arg[2]==7)
        {
            t_config.brp           =4;
            t_config.tseg_1        =15;
            t_config.tseg_2        =4;
            t_config.sjw           =3;
            t_config.triple_sampling=false;

        }
#ifdef DEBUG        
        printf("Send Queue,CAN_START\n");
#endif
        xSend.message_type=START_CAN;
        xQueueSend(xCanQueue,&xSend,portMAX_DELAY);
#ifdef DEGUG        
        printf("Send Queue,CAN_START Over\n");
#endif
    }
    if(arg[0]==0&&arg[1]==1)
    {
        
//    typedef struct {
//    union {
//        struct {
            //The order of these bits must match deprecated message flags for compatibility reasons
//            uint32_t extd: 1;           /**< Extended Frame Format (29bit ID) */
//            uint32_t rtr: 1;            /**< Message is a Remote Frame */
//            uint32_t ss: 1;             /**< Transmit as a Single Shot Transmission. Unused for received. */
//            uint32_t self: 1;           /**< Transmit as a Self Reception Request. Unused for received. */
//            uint32_t dlc_non_comp: 1;   /**< Message's Data length code is larger than 8. This will break compliance with ISO 11898-1 */
//            uint32_t reserved: 27;      /**< Reserved bits */
//        };
        //Todo: Deprecate flags
//        uint32_t flags;                 /**< Deprecated: Alternate way to set bits using message flags */
//    };
//    uint32_t identifier;                /**< 11 or 29 bit identifier */
//    uint8_t data_length_code;           /**< Data length code */
//    uint8_t data[TWAI_FRAME_MAX_DLC];    /**< Data bytes (not relevant in RTR frame) */
//} twai_message_t;*/
        //uint32_t extd;
         
        tx_can_message.extd=arg[2];
        tx_can_message.rtr =arg[3];
#ifdef DEBUG        
        for(int i=0;i<len;i++)
        {
            printf("arg[%d]=%2.2x\n",i,arg[i]);
        }
#endif        
        //tx_can_message.ss=1;

        for(int i=0;i<len-8;i=i+1)
        {
            tx_can_message.data[i]=arg[i+8];
            //printf("data[%d]=%2.2x\n",i,arg[i+3]);
        }    
        tx_can_message.identifier=arg[4]+(arg[5]<<8)+(arg[6]<<16)+(arg[7]<<24);
        tx_can_message.data_length_code=len-8;
        xSend.message_type=START_SEND_ONCE;
        xQueueSend(xCanQueueSend,&xSend,portMAX_DELAY);
        //tx_can_message={.identifier = 2, .data_length_code = 8,
        //                             .data = {0xaa, 0xff , 0 , 0 ,0 ,0 ,0 ,0}};
        //twai_message_t rx_can_message={.identifier = 2, .data_length_code = 8,
        //                             .data = {0xff, 0xaa , 0 , 0 ,0 ,0 ,0 ,0}};
    }
}