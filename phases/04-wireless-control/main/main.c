#include <stdio.h>
#include "freertos/FreeRTOS.h" //
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#define BUTTON_GPIO 14
#define LED_GPIO 25
#define RELAY_GPIO 26
#define ESP_WIFI_SSID      "ESP32-SMARTPLUG"
#define ESP_WIFI_PASS      "123456789"
//#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN


/* Our URI handler function to be called during GET /uri request */
esp_err_t get_handler(httpd_req_t *req)
{
    /* Send a simple response */
    const char* resp = "<html>\n"
"  <head><title>Smart Plug</title></head>\n"
"  <body>\n"
"    <h1>Relay Control</h1>\n"
"    <button onclick=\"fetch('/toggle')\">Toggle Relay</button>\n"
"  </body>\n"
"</html>\n";
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
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    printf("SERVER CONFIG GENERATED\n"); //Debug

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /*Variables for initiliasing, setting, and configuring Wifi-Mode */
    wifi_init_config_t wifi_config_init = WIFI_INIT_CONFIG_DEFAULT(); //Initiliase and allocate resources for Wi-Fi. Start Wi-Fi task
    wifi_mode_t mode = WIFI_MODE_AP; //Set Wi-Fi mode AP, STA, NAN;
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            .channel = 1,
            .max_connection = 3,
            .authmode = WIFI_AUTH_WPA2_PSK,
        },
    }; // Configuration for Wi-Fi credentials

    /*If ommited => E (533) wifi_init: Failed to deinit Wi-Fi driver (0x3001) && AP not accessible
    The Non Volatile Storage is a key-value storage system for the data that must survive if powered off
    such as Wi-Fi configuration, Wi-Fi pairing keys*/
    nvs_flash_init(); 
    esp_netif_init(); //Initialise the network layer abstraction with a TCP/IP stack under the hood
    esp_event_loop_create_default(); //Loop for accepting/closing connections
    esp_netif_create_default_wifi_ap();  // or wifi_sta
    esp_wifi_init(&wifi_config_init); //Initialise the Wi-Fi driver with supplied config
    esp_wifi_set_mode(WIFI_MODE_AP); // Set Wi-Fi mode to Access Point 
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config); // Configure Wi-Fi
    esp_wifi_start(); // Start the Wi-Fi subsystem and make AP visible
    
                     

    /* Start the httpd server */
    if (httpd_start(&server, &server_config) == ESP_OK) {
        /* Register URI handlers */
        printf("SERVER STARTED\n");
        httpd_register_uri_handler(server, &uri_get);
        //httpd_register_uri_handler(server, &uri_post);
    }
    /* If server failed to start, handle will be NULL */
    printf("SERVER RETURNED!");
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

/* Function for wrapping start_server to a compliant taskCode*/
void server_task(void * pvParameters){
    start_webserver();
    for(;;){
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
}

/*Function for controlling the relay via button*/
void button_task(void * pvParameters){
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
    xTaskCreate(server_task,"server_task",4096,NULL,3,NULL);
    vTaskDelay(pdMS_TO_TICKS(100));
    xTaskCreate(button_task,"button_task",2048,NULL,5,NULL);
    vTaskDelay(pdMS_TO_TICKS(100));
}
