#include <stdio.h>
#include "freertos/FreeRTOS.h" //
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_http_server.h"

#define BUTTON_GPIO 14
#define LED_GPIO 25
#define RELAY_GPIO 26

/* Our URI handler function to be called during GET /uri request */
esp_err_t get_handler(httpd_req_t *req)
{
    /* Send a simple response */
    const char resp[] = "URI GET Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}


/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri      = "/uri",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};


/* Function for starting the webserver */
httpd_handle_t start_webserver(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
        //httpd_register_uri_handler(server, &uri_post);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

/* Function for stopping the webserver */
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}

/*Function for controlling the relay via button*/
void button_toggle(void * pvParameters){
    int cur_button_state = 0;
    int last_button_state = 0;
    int led_state = 0;
    
    for(;;){
        cur_button_state = gpio_get_level(BUTTON_GPIO);
        if(cur_button_state == 0 && last_button_state == 1){
            led_state = !led_state;
            gpio_set_level(LED_GPIO,led_state);
            gpio_set_level(RELAY_GPIO,led_state);
            vTaskDelay(pdMS_TO_TICKS(10));
            printf("Relay toggled to: %d\n", led_state);
        }
    last_button_state = cur_button_state;
    vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void){
    gpio_reset_pin(BUTTON_GPIO);
    gpio_reset_pin(RELAY_GPIO);
    gpio_reset_pin(LED_GPIO);

    gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(RELAY_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);

    
    /* BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
                         const char * const pcName,
                         const configSTACK_DEPTH_TYPE uxStackDepth,
                         void *pvParameters,
                         UBaseType_t uxPriority,
                         TaskHandle_t *pxCreatedTask
                       );
    */
    xTaskCreate(button_toggle,"button_task",2048,NULL,5,NULL);
    vTaskDelay(pdMS_TO_TICKS(100));
}
