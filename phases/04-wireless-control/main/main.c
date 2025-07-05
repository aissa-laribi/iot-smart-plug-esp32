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
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
//#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN
static  int RELAY_STATE = 0;

/*Function for controlling the relay via browser
void browser_toggle(){
    printf("BROWSER TOGGLE\n");
    int current_relay_state = 0;
    int last_relay_state = 0;
    int led_state = 0;

    for(;;){
        current_relay_state = gpio_get_level(RELAY_GPIO);
        if(current_relay_state == 0 && last_relay_state == 1){
            led_state = !led_state;
            gpio_set_level(RELAY_GPIO, led_state);
            gpio_set_level(LED_GPIO, led_state);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
        last_relay_state = current_relay_state;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}*/

/* Our URI handler function to be called during GET /uri request */
esp_err_t get_handler(httpd_req_t *req)
{
        const char* resp = "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<meta charset='UTF-8'>\n"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
        "<title>Remote Control</title>\n"
        "</head>\n"
        "<body>\n"
        "<div class='container'>\n"
        "<nav>\n"
            "<h1>ESP32 Smart Plug</h1>\n"
        "</nav>\n"
        "<div id='col1'></div>\n"
        "<div id='col2'>\n"
        "<button type='button' onclick='httpToggle()'>\n"
        "<svg xmlns='http://www.w3.org/2000/svg' width='300' height='300' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2' stroke-linecap='round' stroke-linejoin='round' class='feather feather-power'><path d='M18.36 6.64a9 9 0 1 1-12.73 0'></path><line x1='12' y1='2' x2='12' y2='12'></line></svg>\n"
        "</button>\n"
        "</div>\n"
        "<div id='col3'></div>"
        "</div>"
        "</body>\n"
        "</html>\n"
        "<style>\n"
            "* {margin: 0;padding: 0;box-sizing: border-box;}\n"
            ".container {\n"
            "display: grid;\n"
            "height: 100vh;\n"

            "grid-template-columns: 3fr 4fr 3fr;\n"
            "grid-template-rows: 1fr 9fr;\n"
            "grid-template-areas:\n" 
                "'nav nav nav'\n" 
                "'col1 col2 col3'\n"
                ";\n"
                    "}\n"

            "h1 {\n"
                "color: #b9abab;\n"
                "text-align: center;\n"
                "font-family: Verdana, Geneva, Tahoma, sans-serif;\n"
                "font-weight: bold;\n"

            "}\n"
            "nav {\n"
                "background-color: rgb(255, 255, 255);\n"
                "grid-area: nav;\n"
            "}\n"
            "#col1{\n"
                "background-color: aliceblue;\n"
                "grid-area: col1;\n"
            "}\n"
            "#col2{\n"
                "background-color:aliceblue;\n"
                "grid-area: col2;\n"
            "}\n"
            "#col3{\n"
                "background-color: aliceblue;\n"
                "grid-area: col3;\n"
            "}\n"
            "button {\n"
                "background-color: aliceblue;\n"
                "border-radius: 10em;\n"
                "box-shadow: 0.1em 0.7em black;\n"
                "margin: 2em;\n"
                "width: -webkit-fill-available;\n"
            "}\n"
            "button.active {\n"
                "box-shadow: 0 5px #666;\n"
                "transform: translateY(0.4em);\n"
                "svg{\n"
                    "stroke: lime;\n"
                "}\n"
            "}\n"
            "@media screen and (max-width: 550px){\n"
                ".container{\n"
                    "grid-template-columns: 0 1fr 0;\n"
                    "grid-template-rows: 0.1fr 10fr;\n"
                    "grid-template-areas:\n"
                    "'nav nav nav'\n"
                    "'col1 col2 col3'\n"
                    ";\n"
                "}\n"
            "}\n"
        "</style>\n"
        "<script>\n"
            "const button = document.querySelector('button');\n"
            "button.addEventListener('click', () => {\n"
            "button.classList.toggle('active');});\n"

            "function updateName() {\n"
            "if(button.textContent === 'SWITCH ON'){\n"
                "button.textContent = 'SWITCH OFF';\n"
            "} else if(button.textContent === 'SWITCH OFF') {\n"
                "button.textContent = 'SWITCH ON';\n"
            "}\n"
            "}\n"
            "async function httpToggle(){\n"
                "const response = await fetch('/uri', {\n"
                "method: 'POST',\n"
                "});\n"
            "}\n"
        "</script>\n";
        httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }
/* Our URI handler function to be called during POST /uri request */
esp_err_t post_handler(httpd_req_t *req)
{
    printf("POST HANDLER CALLED\n");
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    char content[100];

    /* Truncate if content length larger than the buffer */
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    printf("Metod: %d", req->method);
    

    /* Send a simple response */
    const char resp[] = "URI POST Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    switch(RELAY_STATE){
        case 0:
            gpio_set_level(RELAY_GPIO,1);
            gpio_set_level(LED_GPIO,1);
            RELAY_STATE = 1;
            break;
        case 1:
            gpio_set_level(RELAY_GPIO,0);
            gpio_set_level(LED_GPIO,0);
            RELAY_STATE = 0;
            break;
        default:
            break;
    }
    return ESP_OK;
}

/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri      = "/uri",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

/* URI handler structure for POST /uri */
httpd_uri_t uri_post = {
    .uri      = "/uri",
    .method   = HTTP_POST,
    .handler  = post_handler,
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
        httpd_register_uri_handler(server, &uri_post);
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
            RELAY_STATE = ! RELAY_STATE;
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
    /*xTaskCreate(browser_toggle,"browser_toggle",2048,NULL,5,NULL);
    vTaskDelay(pdMS_TO_TICKS(100));*/
    xTaskCreate(button_task,"button_task",2048,NULL,5,NULL);
    vTaskDelay(pdMS_TO_TICKS(100));
}
