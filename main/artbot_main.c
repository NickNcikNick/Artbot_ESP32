#include <stdio.h>
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "wifi_ap.h"
#include "sd_card.h"
#include "websocket.h"
#include "http_post.h"

static const char *TAG = "artbot_main";

void sd_card_task(void *pvParameters) {
    ESP_LOGI("SD_CARD_TASK", "Initializing SD Card...");
    esp_err_t ret = sd_card_initialize();
    if (ret != ESP_OK) {
        ESP_LOGE("SD_CARD_TASK", "SD Card initialization failed!");
    } else {
        ESP_LOGI("SD_CARD_TASK", "SD Card initialized successfully");
    }
    vTaskDelete(NULL);  // Delete task after completion
}

void websocket_task(void *pvParameters) {
    ESP_LOGI("WEBSOCKET_TASK", "Starting Unified HTTP Server...");
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    if (httpd_start(&server, &config) == ESP_OK) {
        register_websocket_handler(server);
        register_http_post_handler(server);
        ESP_LOGI("WEBSOCKET_TASK", "WebSocket and HTTP POST registered.");
    } else {
        ESP_LOGE("WEBSOCKET_TASK", "Failed to start HTTP server!");
    }
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // Avoid exiting main task
    }
}

void app_main(void) {
    esp_err_t ret;

    //initialize nvs flash
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS initialized successfully");

    //Initialize Wifi AP
    ESP_LOGI(TAG, "Initializing Wi-Fi AP");
    wifi_init_softap();

    // Create FreeRTOS tasks
    xTaskCreate(websocket_task, "WebSocket Task", 4096, NULL, 5, NULL);
    xTaskCreate(sd_card_task, "SD Card Task", 4096, NULL, 5, NULL);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // Keeps the main function running
    }
}