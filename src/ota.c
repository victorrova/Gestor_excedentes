#include "ota.h"

ESP_EVENT_DECLARE_BASE(MACHINE_EVENTS);

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(__FUNCTION__, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(__FUNCTION__, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(__FUNCTION__, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(__FUNCTION__, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(__FUNCTION__, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(__FUNCTION__, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(__FUNCTION__, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(__FUNCTION__, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}
void Ota_task(void *pvParameter)
{
    char *url = (char*)pvParameter;
    ESP_LOGI(__FUNCTION__, "Inicio de Ota desde url: %s",url);
    esp_http_client_config_t config = {
        .url = url, 
        .event_handler = _http_event_handler,
        .keep_alive_enable = true,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    ESP_LOGI(__FUNCTION__, "Attempting to download update from %s", config.url);
    esp_err_t ret = esp_https_ota(&ota_config);
    if (ret == ESP_OK) {
        ESP_LOGI(__FUNCTION__, "OTA Succeed...");
        esp_event_post(MACHINE_EVENTS,MACHINE_OTA_OK,NULL,0,portMAX_DELAY);
    } else {
        ESP_LOGE(__FUNCTION__, "Firmware upgrade failed");
        esp_event_post(MACHINE_EVENTS,MACHINE_OTA_FAIL,NULL,0,portMAX_DELAY);
    }

    vTaskDelete(NULL);
}
