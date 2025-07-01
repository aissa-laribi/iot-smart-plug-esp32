#include <stdio.h>
#include "freertos/FreeRTOS.h" //
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define BUTTON_GPIO 14
#define LED_GPIO 25
#define RELAY_GPIO 26

void app_main(void){
    gpio_reset_pin(BUTTON_GPIO);
    gpio_reset_pin(RELAY_GPIO);
    gpio_reset_pin(LED_GPIO);

    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
    int cur_button_state = 0;
    int last_button_state = 0;
    int led_state = 0;

    while(1){
        cur_button_state = gpio_get_level(BUTTON_GPIO);
        if(cur_button_state == 0 && last_button_state == 1){
            led_state = !led_state;
            gpio_set_level(LED_GPIO,led_state);
            gpio_set_level(RELAY_GPIO,led_state);
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        last_button_state = cur_button_state;
    }
}
