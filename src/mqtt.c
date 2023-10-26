/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/



#include "mqtt.h"


static const char *TAG = "mqtt_example";

static esp_mqtt_client_handle_t client;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}


esp_err_t mqtt_init(void)
{
    esp_mqtt_client_config_t mqtt_cfg ={0};
    mqtt_cfg.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
    uint32_t port = 0;
    char *uri = (char*)malloc(sizeof(char));
    char *broker_ip=(char*)malloc(sizeof(char));
    char *id = (char*)malloc(sizeof(char));
    char *user = (char*)malloc(sizeof(char));
    char *password = (char*)malloc(sizeof(char));
    size_t broker_ip_len= storage_get_size("mqtt_host");
    if(broker_ip_len == 0)
    {
        size_t broker_uri_len = storage_get_size("mqtt_uri");
        if(broker_uri_len == 0)
        {
            return ESP_FAIL;
        }
        else
        {
            uri = (char*)realloc(uri,sizeof(char) * broker_uri_len);
            ESP_MALLOC_CHECK(uri);
            ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"mqtt_uri",uri,&broker_uri_len));
            mqtt_cfg.broker.address.uri = uri;

        }

    }
    else
    {
        broker_ip  = (char*)realloc(broker_ip,sizeof(char) * broker_ip_len);
        ESP_MALLOC_CHECK(broker_ip);
        ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"mqtt_host",broker_ip,&broker_ip_len));

        mqtt_cfg.broker.address.hostname = broker_ip;
        ESP_ERROR_CHECK(storage_load(NVS_TYPE_U32,"mqtt_port",&port,NULL));
        if(port == 0)
        {
            ESP_LOGE(__FUNCTION__,"puerto erroneo");
            return ESP_FAIL;
        }
    }
    size_t id_len = storage_get_size("mqtt_id");
    if(id_len > 0)
    {
        id = (char*)realloc(id,sizeof(char) * id_len);
        ESP_MALLOC_CHECK(id);
        ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"mqtt_id",id,&id_len));
        mqtt_cfg.credentials.client_id = id;
    }
    size_t user_len = storage_get_size("mqtt_user");
    if(user_len > 0)
    {
        user = (char*)realloc(user,sizeof(char) * user_len);
        ESP_MALLOC_CHECK(user);
        ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"mqtt_user",user,&user_len));
        size_t pass_len = storage_get_size("mqtt_pass");
        if(pass_len >0)
        {
            password = (char*)realloc(password,sizeof(char) * pass_len);
            ESP_MALLOC_CHECK(password);
            ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"mqtt_pass",password,&pass_len));
            mqtt_cfg.credentials.username = user;
            mqtt_cfg.credentials.authentication.password = password;
        }
    }
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    if(uri != NULL)
    {
        free(uri);
    }
    if(broker_ip != NULL)
    {
        free(broker_ip);
    }
    if(id != NULL)
    {
        free(id);
    }
    if(user != NULL)
    {
        free(user);
    }
    if(password != NULL)
    {
        free(password);
    }
    return ESP_OK;

}

