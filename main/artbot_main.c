#include <stdio.h>
#include "esp_err.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_http_client.h"
#include "wifi_ap.h"
#include "sd_card.h"
#include "websocket.h"
#include "http_post.h"
#include "esp_heap_caps.h"


#define BUTTON_GPIO 8
#define CHUNK_SIZE 4096

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

void button_upload_task(void *pvParameter) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    int was_pressed = 0;

    while (1) {
        int pressed = gpio_get_level(BUTTON_GPIO) == 0;

        if (pressed && !was_pressed) {
            ESP_LOGI("BUTTON", "Button Pressed, uploading test.svg...");
            was_pressed = 1;  // Lock until button released

            FILE *file = fopen("/sdcard/test.svg", "rb");
            if (file == NULL) {
                ESP_LOGE("UPLOAD", "Could not open /sdcard/test.svg");
            } else {
                fseek(file, 0, SEEK_END);
                long len = ftell(file);
                rewind(file);

                if (len <= 0) {
                    ESP_LOGE("UPLOAD", "test.svg is empty or invalid");
                    fclose(file);
                } else {
                    esp_http_client_config_t config = {
                        .url = "http://192.168.4.2:8080/upload",
                        .method = HTTP_METHOD_POST,
                    };
                    esp_http_client_handle_t client = esp_http_client_init(&config);

                    esp_err_t err = esp_http_client_open(client, len);
                    if (err != ESP_OK) {
                        ESP_LOGE("UPLOAD", "Failed to open HTTP connection: %s", esp_err_to_name(err));
                    } else {
                        uint8_t *chunk = malloc(CHUNK_SIZE);
                        if (!chunk) {
                            ESP_LOGE("UPLOAD", "Chunk buffer allocation failed");
                        } else {
                            size_t total_sent = 0;
                            size_t read_size;
                            while ((read_size = fread(chunk, 1, CHUNK_SIZE, file)) > 0) {
                                esp_err_t write_err = esp_http_client_write(client, (const char *)chunk, read_size);
                                if (write_err < 0) {
                                    ESP_LOGE("UPLOAD", "Write failed: %s", esp_err_to_name(write_err));
                                    break;
                                }
                                total_sent += read_size;
                            }

                            ESP_LOGI("UPLOAD", "Total bytes sent: %d", (int)total_sent);
                            free(chunk);
                        }

                        esp_http_client_fetch_headers(client);
                        esp_http_client_close(client);
                    }

                    esp_http_client_cleanup(client);
                    fclose(file);
                }
            }

            ESP_LOGI("BUTTON", "Upload complete, waiting for next press...");
        }

        if (!pressed && was_pressed) {
            was_pressed = 0;  // Button released
        }

        vTaskDelay(pdMS_TO_TICKS(50));  // Debounce delay
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
    xTaskCreate(button_upload_task, "Button Upload Task", 8192, NULL, 5, NULL);


    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // Keeps the main function running
    }
}