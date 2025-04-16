#include <esp_event.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <string.h>
#include <stdlib.h>

static const char *TAG = "ws_echo_server";
/*
// WebSocket handler
static esp_err_t echo_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "WebSocket handler triggered with method: %d", req->method);
    ESP_LOGI(TAG, "Echoing message back1...");
    if (req->method == HTTP_GET) {
        return ESP_OK;
    }
    ESP_LOGI(TAG, "Echoing message back2...");
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    ESP_LOGI(TAG, "Echoing message back3...");
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    // Get frame length
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    ESP_LOGI(TAG, "WebSocket frame received of length: %d", ws_pkt.len);
    ESP_LOGI(TAG, "Echoing message back4...");

    if (ret != ESP_OK) {
        return ret;
    }

    if (ws_pkt.len) {
        buf = calloc(1, ws_pkt.len + 1);
        if (!buf) {
            ESP_LOGE(TAG, "Memory allocation failed");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            free(buf);
            return ret;
        }
        //Print message to console
        ESP_LOGI(TAG, "Received WebSocket Message: %s", (char *)ws_pkt.payload);
    }

    // Echo back the received message
    ret = httpd_ws_send_frame(req, &ws_pkt);
    ESP_LOGI(TAG, "WebSocket frame received of length: %d", ws_pkt.len);
    ESP_LOGI(TAG, "Echoing message back...");

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send WebSocket frame: %d", ret);
    }

    free(buf);
    return ret;
}
*/

static esp_err_t echo_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "WebSocket handler triggered with method: %d", req->method);

    if (req->method == HTTP_GET) {
        // This is the handshake request
        ESP_LOGI(TAG, "Handshake completed.");
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get frame length: %d", ret);
        return ret;
    }

    if (ws_pkt.len > 0) {
        buf = calloc(1, ws_pkt.len + 1);
        if (!buf) {
            ESP_LOGE(TAG, "Memory allocation failed");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to receive frame: %d", ret);
            free(buf);
            return ret;
        }

        ESP_LOGI(TAG, "Received WebSocket Message: %s", (char *)ws_pkt.payload);
        ret = httpd_ws_send_frame(req, &ws_pkt);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send WebSocket frame: %d", ret);
        }

        free(buf);
        return ret;
    }

    ESP_LOGW(TAG, "Received empty WebSocket message.");
    return ESP_OK;
}

// WebSocket URI handler
static const httpd_uri_t ws = {
    .uri        = "/ws",
    .method     = HTTP_GET,
    .handler    = echo_handler,
    .user_ctx   = NULL,
    .is_websocket = true
};

// register WebSocket server
void register_websocket_handler(httpd_handle_t server)
{
    httpd_register_uri_handler(server, &ws);
}

