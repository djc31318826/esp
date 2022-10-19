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
#include "lvgl_helpers.h"
#include "../managed_components/lvgl__lvgl/examples/lv_examples.h"
#include "../managed_components/lvgl__lvgl/lvgl.h"

static void lv_tick_task(void *arg);
void guiTask(void *pvParameter);
static void create_demo_application(void);
