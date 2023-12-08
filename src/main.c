#include "storage.h"
#include "wifi.h"
#include "mqtt.h"
#include "pid.h"
#include "dimmer.h"
#include "hlw8032.h"
#include "http_server_app.h"
#include "event.h"

extern EventGroupHandle_t Bits_events;

static hlw8032_t  hlw_meter;
esp_err_t Meter_init(void)
{
   esp_err_t err = ESP_FAIL;
   err = hlw8032_serial_begin(&hlw_meter,2,16,256);
   hlw8032_set_I_coef_from_R(&hlw_meter, 0.001);
   hlw8032_set_V_coef_from_R(&hlw_meter, 1880000, 1000);
   return err;
}

void Machine_init(void)
{
    esp_err_t err = ESP_FAIL;
    err = Event_init();
    err = storage_init();
    err  = queue_start();
    err  = termistor_init();
    err  = Fan_init();
    err  = Wifi_init();
    err = mqtt_init();
    err = Meter_init();
    if(err == ESP_OK)
    {
        xEventGroupSetBits(Bits_events, MACHINE_CONF_OK);
    }

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
                ESP_ERROR_CHECK_WITHOUT_ABORT(queue_to_mqtt_publish(msg));
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
                    else if( Find_Key(payload,"wifi"))
                    {
                        ESP_LOGE(__FUNCTION__, "Recibido datos wifi");
                    }
                    //cJSON_Delete(payload);
                    
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
void  Keepalive_Task(void *params)
{
    cJSON *root = cJSON_CreateObject();
    int state_gestor = 0;
    float temp = 0.0;
    float voltage = 0.0;
    float intensidad = 0.0 ;
    float P_activa = 0.0;
    float P_appa = 0.0;
    float factor_p = 0.0;
    esp_err_t err = ESP_FAIL;
    while(1)
    {
        msg_queue_t msg = queue_receive(DIMMER_TX,100);
        if(msg.len_msg >0 && strcmp(msg.topic,"level")== 0)
        {
            state_gestor =atoi(msg.msg);
        }
        err = hlw8032_read(&hlw_meter);
        temp = temp_termistor();
        if(err == ESP_OK)
        {
            voltage = hlw8032_get_V(&hlw_meter);
            intensidad = hlw8032_get_I(&hlw_meter);
            P_activa = hlw8032_get_P_active(&hlw_meter);
            P_appa = hlw8032_get_P_apparent(&hlw_meter);
            factor_p = hlw8032_get_P_factor(&hlw_meter);
        }
        
        vTaskDelay(30000/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main(void)
{
    
    Machine_init();
    /*ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"ssid", "CASA"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"password","k3rb3r0s"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_U32,"mqtt_port", (uint32_t)1883));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_host", "192.168.0.100"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_sub", "prueba/prueba"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_pub", "prueba/prueba"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"url_inverter", "http://192.168.1.39/measurements.xml"));*/
    Wifi_run(WIFI_MODE_STA); 
    dimmer_init();
    xTaskCreate(&Com_Task,"task1",10000,NULL,3,NULL);
    http_server_start();
    
    
}

