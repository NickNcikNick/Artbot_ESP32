#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "esp_http_server.h"

// Function to start the WebSocket server
void register_websocket_handler(httpd_handle_t server);

#endif // WEBSOCKET_H