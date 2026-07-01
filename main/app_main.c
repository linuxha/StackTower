#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "nvs_flash.h"

#include "stacktower_core.h"
#include "stacktower_hw.h"

#define STACKTOWER_TICK_MS 50U
#define STACKTOWER_WIFI_CONNECTED_BIT BIT0

static const char *TAG = "stacktower";

static stacktower_controller_t s_controller;
static SemaphoreHandle_t s_controller_mutex;
static EventGroupHandle_t s_wifi_event_group;
static esp_mqtt_client_handle_t s_mqtt_client;
static bool s_mqtt_started;

static int64_t stacktower_timestamp_now(void)
{
    time_t now;

    time(&now);
    return (int64_t)now;
}

static esp_err_t stacktower_apply_outputs_locked(void)
{
    uint8_t brightness[STACKTOWER_LED_COUNT];

    stacktower_controller_copy_brightness(&s_controller, brightness);
    return stacktower_hw_apply(brightness);
}

static void stacktower_publish_message(const char *payload)
{
    if ((s_mqtt_client == NULL) || (payload == NULL)) {
        return;
    }

    esp_mqtt_client_publish(s_mqtt_client, CONFIG_STACKTOWER_MQTT_STATE_TOPIC, payload, 0, 1, 0);
}

static void stacktower_publish_state(void)
{
    char payload[STACKTOWER_STATE_JSON_MAX_LEN];

    if (xSemaphoreTake(s_controller_mutex, portMAX_DELAY) != pdTRUE) {
        return;
    }

    if (!stacktower_state_format_json(&s_controller, stacktower_timestamp_now(), payload, sizeof(payload))) {
        xSemaphoreGive(s_controller_mutex);
        ESP_LOGE(TAG, "Failed to build state JSON");
        return;
    }

    xSemaphoreGive(s_controller_mutex);
    stacktower_publish_message(payload);
}

static void stacktower_publish_error(const char *message)
{
    char payload[STACKTOWER_ERROR_JSON_MAX_LEN];

    if (!stacktower_error_format_json(message, stacktower_timestamp_now(), payload, sizeof(payload))) {
        ESP_LOGE(TAG, "Failed to build error JSON");
        return;
    }

    stacktower_publish_message(payload);
}

static void stacktower_handle_command(const char *payload)
{
    char error_message[64];
    stacktower_command_t command;
    esp_err_t result;

    if (!stacktower_command_parse(payload, &command, error_message, sizeof(error_message))) {
        ESP_LOGW(TAG, "Rejected command '%s': %s", payload, error_message);
        stacktower_publish_error(error_message);
        return;
    }

    if (xSemaphoreTake(s_controller_mutex, portMAX_DELAY) != pdTRUE) {
        return;
    }

    stacktower_controller_apply_command(&s_controller, &command);
    result = stacktower_apply_outputs_locked();
    xSemaphoreGive(s_controller_mutex);

    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Failed to apply outputs: %s", esp_err_to_name(result));
        stacktower_publish_error("failed to apply GPIO outputs");
        return;
    }

    ESP_LOGI(TAG, "Applied command: %s %s",
             command.applies_to_all ? "all" : stacktower_led_name(command.led),
             stacktower_mode_name(command.mode));
    stacktower_publish_state();
}

static void stacktower_tick_task(void *arg)
{
    TickType_t last_wake = xTaskGetTickCount();

    (void)arg;

    while (true) {
        esp_err_t result;

        if (xSemaphoreTake(s_controller_mutex, portMAX_DELAY) == pdTRUE) {
            stacktower_controller_tick(&s_controller, STACKTOWER_TICK_MS);
            result = stacktower_apply_outputs_locked();
            xSemaphoreGive(s_controller_mutex);

            if (result != ESP_OK) {
                ESP_LOGE(TAG, "LED update failed: %s", esp_err_to_name(result));
            }
        }

        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(STACKTOWER_TICK_MS));
    }
}

static void stacktower_mqtt_event(void *handler_args,
                                  esp_event_base_t base,
                                  int32_t event_id,
                                  void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    (void)handler_args;
    (void)base;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT connected");
        esp_mqtt_client_subscribe(s_mqtt_client, CONFIG_STACKTOWER_MQTT_CMD_TOPIC, 1);
        stacktower_publish_state();
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "MQTT disconnected");
        break;

    case MQTT_EVENT_DATA:
        if ((event->topic_len == (int)strlen(CONFIG_STACKTOWER_MQTT_CMD_TOPIC)) &&
            (strncmp(event->topic, CONFIG_STACKTOWER_MQTT_CMD_TOPIC, event->topic_len) == 0)) {
            char payload[64];
            int payload_len = event->data_len;

            if (payload_len >= (int)sizeof(payload)) {
                payload_len = sizeof(payload) - 1;
            }

            memcpy(payload, event->data, (size_t)payload_len);
            payload[payload_len] = '\0';
            stacktower_handle_command(payload);
        }
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "MQTT transport error");
        break;

    default:
        break;
    }
}

static void stacktower_start_mqtt(void)
{
    esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = CONFIG_STACKTOWER_MQTT_BROKER_URI,
        .credentials.client_id = CONFIG_STACKTOWER_MQTT_CLIENT_ID,
    };

    if (s_mqtt_started) {
        return;
    }

    s_mqtt_client = esp_mqtt_client_init(&mqtt_config);
    if (s_mqtt_client == NULL) {
        ESP_LOGE(TAG, "Failed to allocate MQTT client");
        return;
    }

    esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, stacktower_mqtt_event, NULL);
    ESP_ERROR_CHECK(esp_mqtt_client_start(s_mqtt_client));
    s_mqtt_started = true;
}

static void stacktower_wifi_event(void *arg,
                                  esp_event_base_t event_base,
                                  int32_t event_id,
                                  void *event_data)
{
    (void)arg;
    (void)event_data;

    if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_START)) {
        esp_wifi_connect();
        return;
    }

    if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_DISCONNECTED)) {
        xEventGroupClearBits(s_wifi_event_group, STACKTOWER_WIFI_CONNECTED_BIT);
        ESP_LOGW(TAG, "WiFi disconnected, retrying");
        esp_wifi_connect();
        return;
    }

    if ((event_base == IP_EVENT) && (event_id == IP_EVENT_STA_GOT_IP)) {
        xEventGroupSetBits(s_wifi_event_group, STACKTOWER_WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "WiFi connected");
        stacktower_start_mqtt();
    }
}

static void stacktower_wifi_start(void)
{
    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    if (CONFIG_STACKTOWER_WIFI_SSID[0] == '\0') {
        ESP_LOGW(TAG, "WiFi SSID not configured; MQTT control is disabled");
        return;
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_config));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &stacktower_wifi_event,
                                                        NULL,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &stacktower_wifi_event,
                                                        NULL,
                                                        NULL));

    strlcpy((char *)wifi_config.sta.ssid, CONFIG_STACKTOWER_WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, CONFIG_STACKTOWER_WIFI_PASSWORD, sizeof(wifi_config.sta.password));

    if (CONFIG_STACKTOWER_WIFI_PASSWORD[0] == '\0') {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

void app_main(void)
{
    esp_err_t result;

    result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);

    s_controller_mutex = xSemaphoreCreateMutex();
    s_wifi_event_group = xEventGroupCreate();
    stacktower_controller_init(&s_controller);

    result = stacktower_hw_init();
    ESP_ERROR_CHECK(result);

    if (xSemaphoreTake(s_controller_mutex, portMAX_DELAY) == pdTRUE) {
        ESP_ERROR_CHECK(stacktower_apply_outputs_locked());
        xSemaphoreGive(s_controller_mutex);
    }

    xTaskCreate(stacktower_tick_task, "stacktower_tick", 4096, NULL, 5, NULL);
    stacktower_wifi_start();

    ESP_LOGI(TAG, "StackTower ready");
}
