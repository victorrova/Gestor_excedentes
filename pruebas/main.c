#include "storage.h"
#include "wifi.h"
#include "mqtt.h"
#include "pid.h"
#include "dimmer.h"
#include "hlw8032.h"
#include "http_server_app.h"
#include "event.h"
#include "machine.h"
#include "led.h"

extern EventGroupHandle_t Bits_events;
ESP_EVENT_DECLARE_BASE(MACHINE_EVENTS);

void Com_Task(void *pvparams);
void Machine_event_ok_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGI(__FUNCTION__,"launch machine_ok");
    ESP_ERROR_CHECK(Wifi_run(WIFI_MODE_STA));
    http_server_start();
    //led_machine_ok();
}
void Machine_event_fail_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGI(__FUNCTION__,"launch machine_fail");
    //led_fail();
}
void Machine_connect_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGI(__FUNCTION__,"launch machine_connect");
    //led_total_connect();
    xTaskCreate(&Com_Task,"task1",10000,NULL,3,NULL);
}
void Machine_disconnect_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGI(__FUNCTION__,"launch machine_disconnect");
    //led_machine_ok();
    
}

void Machine_init(void)
{
    esp_err_t err = ESP_FAIL;
    err = Event_init();
    //ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_FAIL,&Machine_event_fail_handler,NULL));
    //ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_OK,&Machine_event_ok_handler,NULL));
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[event_init] error fatal en inicio!");
        ESP_ERROR_CHECK(esp_event_post(MACHINE_EVENTS,MACHINE_FAIL,NULL,0,portMAX_DELAY));
    }
    err = led_init();
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[led_init] error fatal en inicio!");
        
    }
    err = storage_init();
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[storage_init] error fatal en inicio!");
        ESP_ERROR_CHECK(esp_event_post(MACHINE_EVENTS,MACHINE_FAIL,NULL,0,portMAX_DELAY));
    }
    err  = queue_start();
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[queue_start] error fatal en inicio!");
        ESP_ERROR_CHECK(esp_event_post(MACHINE_EVENTS,MACHINE_FAIL,NULL,0,portMAX_DELAY));
    }
    err  = termistor_init();
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[termistor_init] error fatal en inicio!");
        ESP_ERROR_CHECK(esp_event_post(MACHINE_EVENTS,MACHINE_FAIL,NULL,0,portMAX_DELAY));
    }
    err  = Fan_init();
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[Fan_init] error fatal en inicio!");
        ESP_ERROR_CHECK(esp_event_post(MACHINE_EVENTS,MACHINE_FAIL,NULL,0,portMAX_DELAY));
    }
    err  = Wifi_init();

    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[Wifi_init] error fatal en inicio!");
        ESP_ERROR_CHECK(esp_event_post(MACHINE_EVENTS,MACHINE_FAIL,NULL,0,portMAX_DELAY));
    }
    err = mqtt_init();
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[mqtt_init] error  en inicio!");
    }
    err = Meter_init();
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[Meter_init] error fatal en inicio!");
    }
    ESP_LOGI(__FUNCTION__,"[OK] inicio correcto!");
    xEventGroupSetBits(Bits_events, MACHINE_STATE_OK);
    ESP_ERROR_CHECK(esp_event_post(MACHINE_EVENTS,MACHINE_OK,NULL,0,portMAX_DELAY));

}


void Handler_battery_register(void)
{

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&dimmer_connect_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,WIFI_EVENT_STA_DISCONNECTED,&dimmer_disconnect_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_MQTT_CONNECT,&Machine_connect_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_MQTT_DISCONNECT,&Machine_disconnect_handler,NULL));
    //ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_FAIL,&Machine_event_fail_handler,NULL));
    //ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_OK,&Machine_event_ok_handler,NULL));
    
}
static esp_err_t stream_pid(cJSON *payload)
{
    esp_err_t err = ESP_FAIL;
    cJSON *stream = cJSON_GetObjectItem(payload,"stream");
    if(cJSON_IsNull(stream))
    {
        ESP_LOGE(__FUNCTION__,"json no stream :(");
        return err;
    }
    if(Find_Key(stream,"pid"))
    {
        cJSON *pid = cJSON_GetObjectItem(stream,"pid");
        if(!cJSON_IsNull(pid))
        {
            if(Find_Key(pid,"kp"))
            {
                cJSON *kp = cJSON_GetObjectItem(pid,"kp");
                float _kp = (float)kp->valuedouble;
                char buff[64];
                int change =snprintf(buff,sizeof(buff),"%f",_kp);
                if(change == sizeof(buff))
                {
                    err = queue_send(DIMMER_RX,buff,"kp",100);
                }
                else
                {
                    ESP_LOGE(__FUNCTION__,"fallo snprintf");
                    err = ESP_FAIL;
                }
            }
            else if(Find_Key(pid,"ki"))
            {
                cJSON *ki = cJSON_GetObjectItem(pid,"ki");
                float _ki = (float)ki->valuedouble;
                char buff[64];
                int change =snprintf(buff,sizeof(buff),"%f",_ki);
                if(change == sizeof(buff))
                {
                    err = queue_send(DIMMER_RX,buff,"ki",100);
                }
                else
                {
                    ESP_LOGE(__FUNCTION__,"fallo snprintf");
                    err = ESP_FAIL;
                }
            }
            else if(Find_Key(pid,"kd"))
            {
                cJSON *kd = cJSON_GetObjectItem(pid,"kd");
                float _kd = (float)kd->valuedouble;
                char buff[64];
                int change =snprintf(buff,sizeof(buff),"%f",_kd);
                if(change == sizeof(buff))
                {
                    err = queue_send(DIMMER_RX,buff,"kd",100);
                }
                else
                {
                    ESP_LOGE(__FUNCTION__,"fallo snprintf");
                    err = ESP_FAIL;
                }
            }
            else if(Find_Key(pid,"min"))
            {
                cJSON *min = cJSON_GetObjectItem(pid,"min");
                float _min = (float)min->valuedouble;
                char buff[64];
                int change =snprintf(buff,sizeof(buff),"%f",_min);
                if(change == sizeof(buff))
                {
                    err = queue_send(DIMMER_RX,buff,"min",100);
                }
                else
                {
                    ESP_LOGE(__FUNCTION__,"fallo snprintf");
                    err = ESP_FAIL;
                }
            }
            else if(Find_Key(pid,"max"))
            {
                cJSON *max = cJSON_GetObjectItem(pid,"max");
                float _max = (float)max->valuedouble;
                char buff[64];
                int change =snprintf(buff,sizeof(buff),"%f",_max);
                if(change == sizeof(buff))
                {
                    err = queue_send(DIMMER_RX,buff,"max",100);
                }
                else
                {
                    ESP_LOGE(__FUNCTION__,"fallo snprintf");
                    err = ESP_FAIL;
                }
            }
        }
        else
        {
            return ESP_FAIL;
        }
    }
    return err;
}
void Com_Task(void *pvparams)
{   ESP_LOGI(__FUNCTION__, "Com_Task");
    while(1)
    {
        msg_queue_t msg; 
        msg = queue_receive(MASTER,portMAX_DELAY);
        if(msg.len_msg >0)
        {
            switch (msg.dest)
            {
            case MQTT_TX:
                {
                    EventBits_t flags = xEventGroupGetBits(Bits_events);
                
                    if(flags & MQTT_CONNECT)
                    {
                        ESP_ERROR_CHECK_WITHOUT_ABORT(queue_to_mqtt_publish(msg));
                    }
                }
                break;
            case  MQTT_RX || WS_RX:
                {
                    cJSON *payload = cJSON_Parse(msg.msg);
                    
                    if(Find_Key(payload,"dimmer"))
                    {
                        float dimmer = 0.0;
                        ESP_ERROR_CHECK_WITHOUT_ABORT(decode_number_payload(payload,"dimmer",&dimmer));
                        char *buff =(char*)pvPortMalloc(sizeof(float));
                        itoa((int)dimmer,buff,10);
                        queue_send(DIMMER_RX,buff,"dimmer",100);
                        vPortFree(buff);

                    }
                    else if(Find_Key(payload,"temperature"))
                    {
                        /* mandar a dimmer cuando el control de temperatura este echo*/
                    }
                    else if (Find_Key(payload,"storage"))
                    {   
                        xTaskCreate(&storage_task,"storage_task",3096,payload,1,NULL);
                    }
                    else if( Find_Key(payload,"stream"))
                    {
                        ESP_ERROR_CHECK_WITHOUT_ABORT(stream_pid(payload));
                    }
                }
                break;
            default:
                msg.count +=1;
                queue_send(msg.dest,msg.msg,msg.topic,portMAX_DELAY);
                break;
            }  
        }
    }
    vTaskDelete(NULL);
}
void Control(void *pvparams)
{
    /*control de la cola*/
    /* causa de reinicio*/
    /*control reinicio*/
    /*salto keep alive*/
}

void app_main(void)
{
    
    //Machine_init();
    //Handler_battery_register();
    //vTaskDelay(1000/portTICK_PERIOD_MS);
    //Wifi_run(WIFI_MODE_STA);
    //ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&dimmer_connect_handler,NULL));
    //ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,WIFI_EVENT_STA_DISCONNECTED,&dimmer_disconnect_handler,NULL));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"ssid", "CASA"));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"password","k3rb3r0s"));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_U32,"mqtt_port", (uint32_t)1883));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_host", "192.168.0.100"));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_sub", "prueba/prueba"));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_pub", "prueba/prueba"));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"url_inverter", "http://192.168.1.39/measurements.xml"));
    termistor_init();
    while(1)
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);
        temp_termistor();

    }
}




