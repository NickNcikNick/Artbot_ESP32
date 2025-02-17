// testing this file

#include <esp_log.h>
#include <esp_http_server.h>
#include <nvs_flash.h>
#include <nvs.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

static const char *REST_TAG = "http_post";
static const char *NVS_NAMESPACE = "storage";  
static const char *NVS_KEY = "file_number";    
#define BUFFER_SIZE 1024  // Buffer size for file reading

// Function to extract filename from Content-Disposition header
static void get_filename_from_headers(httpd_req_t *req, char *filename, size_t size) {
    char content_disposition[128] = {0};

    if (httpd_req_get_hdr_value_str(req, "Content-Disposition", content_disposition, sizeof(content_disposition)) == ESP_OK) {
        char *filename_start = strstr(content_disposition, "filename=");
        if (filename_start) {
            filename_start += 9; // Skip "filename="
            char *quote_start = strchr(filename_start, '"');
            if (quote_start) {
                filename_start = quote_start + 1;
                char *quote_end = strchr(filename_start, '"');
                if (quote_end) {
                    *quote_end = '\0';
                }
            }
            strncpy(filename, filename_start, size - 1);
            filename[size - 1] = '\0';
        }
    }
}

// File Upload Handler
static esp_err_t upload_handler(httpd_req_t *req) {
    char buf[BUFFER_SIZE];
    int received;
    char filename[64] = {0};

    // Extract filename from headers
    get_filename_from_headers(req, filename, sizeof(filename));

    // If filename is missing, generate a default one
    if (strlen(filename) == 0) {
        snprintf(filename, sizeof(filename), "file_default.bin");
    }

    // Construct full file path
    char file_path[128];
    snprintf(file_path, sizeof(file_path), "/sdcard/%s", filename);

    FILE *file = fopen(file_path, "wb");
    if (!file) {
        ESP_LOGE(REST_TAG, "Failed to open file for writing: %s", file_path);
        return ESP_FAIL;
    }

    // Read and write file data, skipping HTTP multipart headers
    bool in_file_data = false;
    while ((received = httpd_req_recv(req, buf, sizeof(buf))) > 0) {
        if (!in_file_data) {
            char *file_start = strstr(buf, "\r\n\r\n"); // Find end of headers
            if (file_start) {
                file_start += 4; // Move past header section
                fwrite(file_start, 1, received - (file_start - buf), file);
                in_file_data = true;
            }
        } else {
            fwrite(buf, 1, received, file);
        }
    }

    fclose(file);
    ESP_LOGI(REST_TAG, "File upload complete: %s", file_path);

    httpd_resp_send(req, "Upload successful!", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// URI Registration
static const httpd_uri_t upload_uri = {
    .uri       = "/upload",
    .method    = HTTP_POST,
    .handler   = upload_handler,
    .user_ctx  = NULL
};

// Register HTTP POST Endpoint
void register_http_post_handler(httpd_handle_t server) {
    httpd_register_uri_handler(server, &upload_uri);
}
