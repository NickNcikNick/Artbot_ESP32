#include <esp_event.h>
#include <esp_log.h>
#include <esp_http_server.h>
#include <string.h>
#include <stdlib.h>

static const char *TAG = "ws_echo_server";

// WebSocket handler
static esp_err_t echo_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        return ESP_OK;
    }

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;

    // Get frame length
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
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
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to send WebSocket frame: %d", ret);
    }

    free(buf);
    return ret;
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

