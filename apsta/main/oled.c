#include "oled.h"



  DRAM_ATTR static const lcd_init_cmd_t ili_init_cmds[]={};
    DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[]={
        {0xAE, {0x00}, 0},
        {0xD5, {0x80}, 1},
        {0xA8, {0x3f}, 1},
        {0xD3, {0x0}, 1},
        {0x40, {0x0}, 0},
        {0x8D, {0x14}, 1},
        {0x20, {0x02}, 1},
        {0xA1, {0x0}, 0},
        {0xC8, {0x0}, 0},
        {0xDA, {0x12}, 1},
        //{0x81, {0xef}, 1},
        {0x81, {0xef}, 1},
        {0xD9, {0xf1}, 1},
        {0xDB, {0x30}, 1},
        {0xA4, {0x0}, 0},
        //{0xA5, {0x0}, 0},
        //{0xA7, {0x0}, 0},
        {0xA6, {0x0}, 0},
        {0xAF, {0x0}, 0},
        {0, {0}, 0xff}
    };

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_cmd(spi_device_handle_t spi, const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void lcd_data(spi_device_handle_t spi, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len==0) return;             //no need to send anything
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=len*8;                 //Len is in bytes, transaction length is in bits.
    t.tx_buffer=data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

uint32_t lcd_get_id(spi_device_handle_t spi)
{
    //get_id cmd
    lcd_cmd(spi, 0x04);

    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length=8*3;
    t.flags = SPI_TRANS_USE_RXDATA;
    t.user = (void*)1;

    esp_err_t ret = spi_device_polling_transmit(spi, &t);
    assert( ret == ESP_OK );

    return *(uint32_t*)t.rx_data;
}

//Initialize the display
void lcd_init(spi_device_handle_t spi)
{
    int cmd=0;
    const lcd_init_cmd_t* lcd_init_cmds;

    //Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);

    //detect LCD type
    uint32_t lcd_id = lcd_get_id(spi);
    int lcd_detected_type = 0;
    int lcd_type;

    lcd_id=1;
    printf("LCD ID: %08X\n", lcd_id);

    if ( lcd_id == 0 ) {
        //zero, ili
        lcd_detected_type = LCD_TYPE_ILI;
        printf("ILI9341 detected.\n");
    } else {
        // none-zero, ST
        lcd_detected_type = LCD_TYPE_ST;
        printf("ST7789V detected.\n");
    }

#ifdef CONFIG_LCD_TYPE_AUTO
    lcd_type = lcd_detected_type;
#elif defined( CONFIG_LCD_TYPE_ST7789V )
    printf("kconfig: force CONFIG_LCD_TYPE_ST7789V.\n");
    lcd_type = LCD_TYPE_ST;
#elif defined( CONFIG_LCD_TYPE_ILI9341 )
    printf("kconfig: force CONFIG_LCD_TYPE_ILI9341.\n");
    lcd_type = LCD_TYPE_ILI;
#endif
    if ( lcd_type == LCD_TYPE_ST ) {
        printf("LCD ST7789V initialization.\n");
        lcd_init_cmds = st_init_cmds;
    } else {
        printf("LCD ILI9341 initialization.\n");
        lcd_init_cmds = ili_init_cmds;
    }

    //Send all the commands
    while (lcd_init_cmds[cmd].databytes!=0xff) {
        printf("cmd=%d,lcd cmd=0x%2.2x\n",cmd,lcd_init_cmds[cmd].cmd);
        lcd_cmd(spi, lcd_init_cmds[cmd].cmd);
        //vTaskDelay(100 / portTICK_RATE_MS);
        //lcd_data(spi, lcd_init_cmds[cmd].data, lcd_init_cmds[cmd].databytes&0x1F);
        for(int i=0;i<(lcd_init_cmds[cmd].databytes&0x1F);i++)
        {
            lcd_cmd(spi, lcd_init_cmds[cmd].data[i]);
            //vTaskDelay(100 / portTICK_RATE_MS);
            printf("data=%d,lcd cmd data=0x%2.2x\n",i,lcd_init_cmds[cmd].data[i]);
        }
        if (lcd_init_cmds[cmd].databytes&0x80) {
            //vTaskDelay(100 / portTICK_RATE_MS);
        }
        cmd++;
    }

    ///Enable backlight
    gpio_set_level(PIN_NUM_BCKL, 0);
}


/* To send a set of lines we have to send a command, 2 data bytes, another command, 2 more data bytes and another command
 * before sending the line data itself; a total of 6 transactions. (We can't put all of this in just one transaction
 * because the D/C line needs to be toggled in the middle.)
 * This routine queues these commands up as interrupt transactions so they get
 * sent faster (compared to calling spi_device_transmit several times), and at
 * the mean while the lines for next transactions can get calculated.
 */
static void send_lines(spi_device_handle_t spi, int ypos, uint16_t *linedata)
{
    esp_err_t ret;
    int x;
    //Transaction descriptors. Declared static so they're not allocated on the stack; we need this memory even when this
    //function is finished because the SPI driver needs access to it even while we're already calculating the next line.
    static spi_transaction_t trans[6];

    //In theory, it's better to initialize trans and data only once and hang on to the initialized
    //variables. We allocate them on the stack, so we need to re-init them each call.
    for (x=0; x<6; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        if ((x&1)==0) {
            //Even transfers are commands
            trans[x].length=8;
            trans[x].user=(void*)0;
        } else {
            //Odd transfers are data
            trans[x].length=8*4;
            trans[x].user=(void*)1;
        }
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }
    trans[0].tx_data[0]=0x2A;           //Column Address Set
    trans[1].tx_data[0]=0;              //Start Col High
    trans[1].tx_data[1]=0;              //Start Col Low
    trans[1].tx_data[2]=(320)>>8;       //End Col High
    trans[1].tx_data[3]=(320)&0xff;     //End Col Low
    trans[2].tx_data[0]=0x2B;           //Page address set
    trans[3].tx_data[0]=ypos>>8;        //Start page high
    trans[3].tx_data[1]=ypos&0xff;      //start page low
    trans[3].tx_data[2]=(ypos+PARALLEL_LINES)>>8;    //end page high
    trans[3].tx_data[3]=(ypos+PARALLEL_LINES)&0xff;  //end page low
    trans[4].tx_data[0]=0x2C;           //memory write
    trans[5].tx_buffer=linedata;        //finally send the line data
    trans[5].length=320*2*8*PARALLEL_LINES;          //Data length, in bits
    trans[5].flags=0; //undo SPI_TRANS_USE_TXDATA flag

    //Queue all transactions.
    for (x=0; x<6; x++) {
        ret=spi_device_queue_trans(spi, &trans[x], portMAX_DELAY);
        assert(ret==ESP_OK);
    }

    //When we are here, the SPI driver is busy (in the background) getting the transactions sent. That happens
    //mostly using DMA, so the CPU doesn't have much to do here. We're not going to wait for the transaction to
    //finish because we may as well spend the time calculating the next line. When that is done, we can call
    //send_line_finish, which will wait for the transfers to be done and check their status.
}


void send_line_finish(spi_device_handle_t spi)
{
    spi_transaction_t *rtrans;
    esp_err_t ret;
    //Wait for all 6 transactions to be done and get back the results.
    for (int x=0; x<6; x++) {
        ret=spi_device_get_trans_result(spi, &rtrans, portMAX_DELAY);
        assert(ret==ESP_OK);
        //We could inspect rtrans now if we received any info back. The LCD is treated as write-only, though.
    }
}


//Simple routine to generate some patterns and send them to the LCD. Don't expect anything too
//impressive. Because the SPI driver handles transactions in the background, we can calculate the next line
//while the previous one is being sent.
/*
void display_pretty_colors(spi_device_handle_t spi)
{
    uint16_t *lines[2];
    //Allocate memory for the pixel buffers
    for (int i=0; i<2; i++) {
        lines[i]=heap_caps_malloc(320*PARALLEL_LINES*sizeof(uint16_t), MALLOC_CAP_DMA);
        assert(lines[i]!=NULL);
    }
    int frame=0;
    //Indexes of the line currently being sent to the LCD and the line we're calculating.
    int sending_line=-1;
    int calc_line=0;

    while(1) {
        frame++;
        //for (int y=0; y<240; y+=PARALLEL_LINES)
        for (int y=0; y<64; y+=PARALLEL_LINES)
        {
            //Calculate a line.
            pretty_effect_calc_lines(lines[calc_line], y, frame, PARALLEL_LINES);
            //Finish up the sending process of the previous line, if any
            if (sending_line!=-1) send_line_finish(spi);
            //Swap sending_line and calc_line
            sending_line=calc_line;
            calc_line=(calc_line==1)?0:1;
            //Send the line we currently calculated.
            send_lines(spi, y, lines[sending_line]);
            //The line set is queued up for sending now; the actual sending happens in the
            //background. We can go on to calculate the next line set as long as we do not
            //touch line[sending_line]; the SPI sending process is still reading from that.
        }
    }
}
*/
spi_device_handle_t spi;
void column_set(unsigned char column)
  {
    lcd_cmd(spi, 0x10|(column>>4));    //?????????????????????????????????
    lcd_cmd(spi, 0x00|(column&0x0f));   //?????????????????????????????????  
    	 
  }

void page_set(unsigned char page)
{
    lcd_cmd(spi, 0xb0+page);
}

void oled_clear(void)
  {
    unsigned char page,column;
    for(page=0;page<8;page++)             //page loop
      { 
          page_set(page);
          column_set(0);	
          uint8_t  send_data[1]={0};
          for(column=0;column<128;column++)	//column loop
            {
              lcd_data(spi,send_data,1);
            }
      }
  }
void oled_full(void)
{
    unsigned char page,column;
    for(page=0;page<8;page++)             //page loop
      { 
        page_set(page);
        column_set(0);	  
        uint8_t  send_data[1]={0xff};
	    for(column=0;column<128;column++)	//column loop
        {
            lcd_data(spi,send_data,1);
        }
    }
}

void oled_test(void)
{
    unsigned char page,column;
    for(page=0;page<8;page++)             //page loop
      { 
        page_set(page);
        column_set(0);	  
        uint8_t  send_data[1]={0xff};
	    for(column=0;column<128;column++)	//column loop
        {
            if(page==0)
            {
            if(column%2==0)
                send_data[0]=0;
            else
                send_data[0]=0xff;
            
            }
            else{
                send_data[0]=0;
            }
            lcd_data(spi,send_data,1);
        }
    }
}

void oled_show(void)
{
    unsigned char page,column;
    int index=0;
    for(page=0;page<8;page++)             //page loop
      { 
        page_set(page);
        column_set(0);	  
        uint8_t  send_data[1]={0xff};
        
	    for(column=0;column<128;column++)	//column loop
        {
            //if(index<sizeof(text_dat)/sizeof(uint8_t))
            //    send_data[0]=text_dat[index++];
            //else
            //    send_data[0]=0xff;
            send_data[0]=frame_buf1[page][column];
            lcd_data(spi,send_data,1);
        }
    }
}

void oled_init()
{
    esp_err_t ret;
    
    spi_bus_config_t buscfg={
        .miso_io_num=PIN_NUM_MISO,
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=PARALLEL_LINES*320*2+8
    };
    spi_device_interface_config_t devcfg={
#ifdef CONFIG_LCD_OVERCLOCK
        .clock_speed_hz=1*1000*1000,           //Clock out at 26 MHz
#else
        .clock_speed_hz=20*1000*1000,           //Clock out at 10 MHz
#endif
        .mode=0,                                //SPI mode 0
        .spics_io_num=PIN_NUM_CS,               //CS pin
        .queue_size=7,                          //We want to be able to queue 7 transactions at a time
        .pre_cb=lcd_spi_pre_transfer_callback,  //Specify pre-transfer callback to handle D/C line
    };
    //Initialize the SPI bus
    //printf("PIN_NUM_MISO=%d,sizeof(text_dat)/sizeof(uint8_t)=%d\n",PIN_NUM_MISO,sizeof(text_dat)/sizeof(uint8_t));
    ret=spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(LCD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
    //Initialize the LCD
    lcd_init(spi);
    set_font(font16x16_gb2312);
}