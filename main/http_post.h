#ifndef HTTP_POST_H
#define HTTP_POST_H

#include "esp_err.h"

// Function to start the HTTP POST server (for file uploads & REST API)
void register_http_post_handler(httpd_handle_t server);

#endif // HTTP_POST_H