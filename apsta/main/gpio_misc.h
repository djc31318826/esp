/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
//io45 HIGH enable power;
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "lwip/sys.h"
#define GPIO_OUTPUT_IO_0    45
#define GPIO_OUTPUT_PIN_SEL  (1ULL<<GPIO_OUTPUT_IO_0)

#define GPIO_INPUT_IO_0     38
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0)
void gpio_misc_init();
void power_on();
void power_off();
int power_button_is_down();
void gpio_input_test(void *p);