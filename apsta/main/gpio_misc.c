#include "gpio_misc.h"
void gpio_misc_init()
{
    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    io_conf.mode=GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    power_on();
}
void power_on()
{
    gpio_set_level(GPIO_OUTPUT_IO_0, 1);

}
void power_off()
{
    gpio_set_level(GPIO_OUTPUT_IO_0, 0);


}
int power_button_is_down()
{
    return (gpio_get_level(GPIO_INPUT_IO_0)==0);
}

void gpio_input_test(void *p)
{
    static int cnt=0;
    while (1)
    {
        // wss_server_send_messages(&handle);
        sys_delay_ms(500);
        //gcnt++;
        //scroll_x();
        if(power_button_is_down())
            cnt++;
        else 
            cnt=0;
        //printf("cnt=%d\n",cnt);
        if(cnt>=6)
            power_off();//gpio_set_level(GPIO_OUTPUT_IO_0, 0);
        
    }
}