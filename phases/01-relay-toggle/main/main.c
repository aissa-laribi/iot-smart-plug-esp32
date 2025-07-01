#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"

#define RELAY_MODULE 26
static const char *TAG = "RelayExample";

void app_main(void)
{
    // Reset and configure the relay pin as output
    gpio_reset_pin(RELAY_MODULE);
    gpio_set_direction(RELAY_MODULE, GPIO_MODE_OUTPUT);

    while (1) {
        ESP_LOGI(TAG, "Relay OFF");
        gpio_set_level(RELAY_MODULE, 0); // Relay OFF
        vTaskDelay(pdMS_TO_TICKS(5000));

        ESP_LOGI(TAG, "Relay ON");
        gpio_set_level(RELAY_MODULE, 1); // Relay ON
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
