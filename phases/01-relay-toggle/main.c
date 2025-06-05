/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define RELAY_MODULE 26


void app_main(void)
{

    gpio_reset_pin(RELAY_MODULE); //Reset GPIO to default state
    gpio_set_direction(RELAY_MODULE, GPIO_MODE_OUTPUT); //GPIO set direction., Configure GPIO mode,such as output_only,input_only,output_and_input
    
    while (1) {
        gpio_set_level(RELAY_MODULE, 0); // OFF
        vTaskDelay(pdMS_TO_TICKS(5000));
        gpio_set_level(RELAY_MODULE, 1); // ON
        vTaskDelay(pdMS_TO_TICKS(5000));
}

}
