#include "storage.h"
#include "wifi.h"
#include "mqtt.h"
#include "pid.h"
#include "dimmer.h"
#include "hlw8032.h"
#include "http_server_app.h"

static const char TAG[] = "main";

void Meter_init(void)
{
    hlw8032_t  hlw_meter;
    ESP_ERROR_CHECK(hlw8032_serial_begin(&hlw_meter,2,17,256));
    hlw8032_set_I_coef_from_R(&hlw_meter, 0.001);
    hlw8032_set_V_coef_from_R(&hlw_meter, 1880000, 1000);

}

void Machine_init(void)
{
    storage_init();
    ESP_ERROR_CHECK(Event_init());
    ESP_ERROR_CHECK(queue_start());
    termistor_init();
    Fan_init();
    Wifi_init();
    //ESP_ERROR_CHECK(mqtt_init());
    Meter_init();
}
static esp_err_t stream_pid(cJSON *payload)
{
    esp_err_t err = ESP_FAIL;
    cJSON *stream = cJSON_GetObjectItem(payload,"stream");
    if(cJSON_IsNull(stream))
    {
        ESP_LOGE(__FUNCTION__,"json no strem :(");
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
{   ESP_LOGI(TAG, "Com_Task");
    while(1)
    {
        msg_queue_t msg; 
        msg = queue_receive(MASTER,portMAX_DELAY);
        if(msg.len_msg >0)
        {
            switch (msg.dest)
            {
            case MQTT_TX:
                ESP_ERROR_CHECK_WITHOUT_ABORT(queue_to_mqtt_publish(msg));
                break;
            case MQTT_RX || WS_RX:
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
                    else if( Find_Key(payload,"wifi"))
                    {
                        ESP_LOGE(TAG, "Recibido datos wifi");
                    }
                    cJSON_Delete(payload);
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


void app_main(void)
{
    
    Machine_init();
    /*ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"ssid", "_____________"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"password","______________"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_U32,"mqtt_port", (uint32_t)1883));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_host", "192.168.0.100"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_sub", "prueba/prueba"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_pub", "prueba/prueba"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"url_inverter", "http://192.168.1.39/measurements.xml"));*/
    Wifi_run(WIFI_MODE_STA); 
    //dimmer_init();
    xTaskCreate(&Com_Task,"task1",10000,NULL,3,NULL);
    storage_get_config();

    http_server_start();
}

