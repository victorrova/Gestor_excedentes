/* MQTT (over TCP) Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/



#include "mqtt.h"


extern EventGroupHandle_t Bits_events;
static esp_mqtt_client_handle_t client;




static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(__FUNCTION__, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_connect_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGI(__FUNCTION__,"conectando con el broker...");
    ESP_ERROR_CHECK(esp_mqtt_client_start(client));
    size_t topic_len = storage_get_size("mqtt_sub");
    if(topic_len >0)
    {
        char *topic = (char*)malloc(sizeof(char) * topic_len);
        ESP_MALLOC_CHECK(topic);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"mqtt_sub",topic,&topic_len));
        esp_mqtt_client_subscribe(client,(const char*)topic, 0);
        free(topic);
    }
    
}

static void mqtt_disconnect_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGI(__FUNCTION__,"desconectando con el broker...");
    if(client != NULL)
    {
        esp_err_t de = esp_mqtt_client_stop(client);
        if(de == ESP_FAIL)
        {
            ESP_LOGE(__FUNCTION__,"cliente mqtt no inicializado");
        }
        else
        {
            xEventGroupClearBits(Bits_events,MQTT_CONNECT);
            ESP_LOGI(__FUNCTION__,"cliente desconectado con exito!");
        }
    }
    
}
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    //ESP_LOGW(__FUNCTION__, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(__FUNCTION__, "MQTT_EVENT_CONNECTED");
        xEventGroupSetBits(Bits_events,MQTT_CONNECT);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(__FUNCTION__, "MQTT_EVENT_DISCONNECTED");
        xEventGroupClearBits(Bits_events,MQTT_CONNECT);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGD(__FUNCTION__, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGD(__FUNCTION__, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGD(__FUNCTION__, "MQTT_EVENT_DATA");
        xEventGroupSetBits(Bits_events,MQTT_ON_MESSAGE);
        char* msg = (char*)malloc(sizeof(char)* event->data_len);
        char* topic = (char*)malloc(sizeof(char)* event->topic_len);
        strncpy(msg,event->data,event->data_len);
        strncpy(topic,event->topic,event->topic_len);
        ESP_ERROR_CHECK(queue_send(MQTT_RX,(const char *)msg,(const char *)topic,portMAX_DELAY));
        if(msg != NULL)
        {
            free(msg);
            free(topic);
        }
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(__FUNCTION__, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(__FUNCTION__, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGD(__FUNCTION__, "Other event id:%d", event->event_id);
        break;
    }
}

esp_err_t mqtt_publish(char * payload,int payload_len, char* topic)
{
    esp_err_t err = ESP_FAIL;
    if(topic != NULL)
    {
        esp_mqtt_client_publish(client,topic,payload,payload_len, 1, 0);
        err = ESP_OK;
    }
    else
    {
        size_t topic_len = storage_get_size("mqtt_pub");
        if(topic_len >0)
        {
            char *_topic = (char*)malloc(sizeof(char) * topic_len);
            ESP_MALLOC_CHECK(_topic);
            err = storage_load(NVS_TYPE_STR,"mqtt_pub",topic,&topic_len);
            esp_mqtt_client_publish(client,_topic,payload,payload_len, 1, 0);
            free(_topic);
        }

    }
    return err;
}
esp_err_t queue_to_mqtt_publish(msg_queue_t msg)
{
    if(msg.len_topic > 0)
    {
        esp_mqtt_client_publish(client,msg.topic,msg.msg,msg.len_msg, 1, 0);
        xEventGroupSetBits(Bits_events,MQTT_ON_MESSAGE);
        return ESP_OK;
    }
    else
    {
        return mqtt_publish(msg.msg,msg.len_msg,NULL);
        xEventGroupSetBits(Bits_events,MQTT_ON_MESSAGE);
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
            ESP_LOGE(__FUNCTION__,"url de broker desconocida");
            return ESP_FAIL;
        }
        else
        {
            uri = (char*)realloc(uri,sizeof(char) * broker_uri_len);
            ESP_MALLOC_CHECK(uri);
            ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"mqtt_uri",uri,&broker_uri_len));
            mqtt_cfg.broker.address.uri = uri;
            ESP_LOGI(__FUNCTION__,"url de broker configurada con exito!");

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
        ESP_LOGI(__FUNCTION__,"mqtt id configurada con exito!");
    }
    else
    {
        ESP_LOGW(__FUNCTION__,"mqtt id por defecto");
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
    else
    {
        ESP_LOGW(__FUNCTION__,"Broker sin credenciales :|");
    }
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&mqtt_connect_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,WIFI_EVENT_STA_DISCONNECTED,&mqtt_disconnect_handler,NULL));
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

