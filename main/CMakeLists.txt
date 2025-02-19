idf_component_register(SRCS "http_post.c" "wifi_ap.c" "sd_card.c" "artbot_main.c" "websocket.c"
                    INCLUDE_DIRS "."
                    REQUIRES json vfs esp_http_server esp_wifi nvs_flash log fatfs sd_card
                    WHOLE_ARCHIVE)

if(CONFIG_EXAMPLE_WEB_DEPLOY_SF)
    set(WEB_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../front/web-demo")
    if(EXISTS ${WEB_SRC_DIR}/dist)
        spiffs_create_partition_image(www ${WEB_SRC_DIR}/dist FLASH_IN_PROJECT)
    else()
        message(FATAL_ERROR "${WEB_SRC_DIR}/dist doesn't exit. Please run 'npm run build' in ${WEB_SRC_DIR}")
    endif()
endif()
