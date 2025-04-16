#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <stdbool.h>
#include "esp_http_server.h"

// Function to start the WebSocket server
void register_websocket_handler(httpd_handle_t server);

#ifdef __cplusplus
extern "C" {
#endif

extern bool camera_single_capture;

#ifdef __cplusplus
}
#endif


#endif // WEBSOCKET_H