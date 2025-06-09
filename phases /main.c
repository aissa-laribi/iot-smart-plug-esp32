#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define RELAY_MODULE 26
#define BUTTON_SWITCH 14


void app_main(void)
{

    gpio_reset_pin(RELAY_MODULE); //Reset GPIO to default state
    gpio_reset_pin(BUTTON_SWITCH);
    gpio_set_direction(RELAY_MODULE, GPIO_MODE_OUTPUT); //GPIO set direction., Configure GPIO mode,such as output_only,input_only,output_and_input
    gpio_set_direction(BUTTON_SWITCH, GPIO_MODE_INPUT); //GPIO set direction., Configure GPIO mode,such as output_only,input_only,output_and_input
    gpio_set_pull_mode(BUTTON_SWITCH, GPIO_PULLUP_ONLY);
    int cur_but_state = 0;
    int last_but_state = 0;
    int ledState = 0;

    while (1) {
        cur_but_state = gpio_get_level(BUTTON_SWITCH);

        if(cur_but_state == 0 && last_but_state == 1){
                ledState = !ledState;
                gpio_set_level(RELAY_MODULE, ledState); // ON
                vTaskDelay(pdMS_TO_TICKS(200));
        }
        last_but_state = cur_but_state;
        }
    }
