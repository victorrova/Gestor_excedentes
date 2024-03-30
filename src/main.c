#include <stdio.h>
#include "config.h"
#include "esp_system.h"
#include <stdio.h>
#include "storage.h"
#include "wifi.h"
#include "mqtt.h"
#include "pid.h"
#include "dimmer.h"
#include "http_server_app.h"
#include "event.h"
#include "machine.h"
#include "led.h"
#include "ota.h"
#include "medidor.h"
#include "config.h"
#include "esp_heap_trace.h"
#include "task_factory.h"

extern EventGroupHandle_t Bits_events;
ESP_EVENT_DECLARE_BASE(MACHINE_EVENTS);
void Com_Task(void *pvparams);

void Machine_event_ok_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGW(__FUNCTION__,"launch machine_ok");
    
    //ESP_ERROR_CHECK(task_create(&Com_Task,"com_task",3,NULL));
    xTaskCreate(&Com_Task,"com_task",6000,NULL,3,NULL);
}
void Machine_wifi_connect(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    led_machine_ok();
    dimmer_init();
}
void Machine_wifi_disconnect(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    dimmer_stop();
    led_off();
}
void Machine_event_fail_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGW(__FUNCTION__,"launch machine_fail");
    led_fail();
}
void Machine_MQTT_connect_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGW(__FUNCTION__,"launch machine_connect");
    led_total_connect();
    
}
void Machine_MQTT_disconnect_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGW(__FUNCTION__,"launch machine_disconnect");
    led_machine_ok();
    
}
void Machine_OTA_OK_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGE(__FILE__,"reboot....");
    led_off();
    esp_restart();
}
void Machine_OTA_FAIL_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    ESP_LOGE(__FILE__,"OTA_fail");
    led_fail();
}   
void Machine_Ap_connect(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data)
{
    dimmer_stop();
    led_AP();
}

void Machine_init(void)
{
    esp_err_t err = ESP_FAIL;
    err = Event_init();
    ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_FAIL,&Machine_event_fail_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_OK,&Machine_event_ok_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_OTA_OK,&Machine_OTA_OK_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_OTA_FAIL,&Machine_OTA_FAIL_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_MQTT_CONNECT,&Machine_MQTT_connect_handler,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(MACHINE_EVENTS,MACHINE_MQTT_DISCONNECT,&Machine_MQTT_disconnect_handler,NULL));
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
    err  = Ap_call_Init();
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[Ap_Button] error fatal en inicio!");
    
    }
     
    err  = Wifi_init();
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT,IP_EVENT_STA_GOT_IP,&Machine_wifi_connect,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,WIFI_EVENT_STA_DISCONNECTED,&Machine_wifi_disconnect,NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT,WIFI_EVENT_AP_START,&Machine_Ap_connect,NULL));
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
#ifndef METER_ENABLE
    err = Meter_init();
    if(err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"[Meter_init] error fatal en inicio!");
    }
#endif
    ESP_LOGI(__FUNCTION__,"[OK] inicio correcto!");
    set_stream_logger(MQTT_TX);
    xEventGroupSetBits(Bits_events, MACHINE_STATE_OK);
    ESP_ERROR_CHECK(esp_event_post(MACHINE_EVENTS,MACHINE_OK,NULL,0,portMAX_DELAY));


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
                printf("buffer = %s change = %d\n",buff,change);
                if(change <= sizeof(buff))
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
                if(change <= sizeof(buff))
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
                if(change <= sizeof(buff))
                {
                    err = queue_send(DIMMER_RX,buff,"kd",100);
                }
                else
                {
                    ESP_LOGE(__FUNCTION__,"fallo sprintf");
                    err = ESP_FAIL;
                }
            }
            else if(Find_Key(pid,"min"))
            {
                cJSON *min = cJSON_GetObjectItem(pid,"min");
                float _min = (float)min->valuedouble;
                char buff[64];
                int change =snprintf(buff,sizeof(buff),"%f",_min);
                if(change <= sizeof(buff))
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
                if(change <= sizeof(buff))
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
{   
    
    ESP_LOGW(__FUNCTION__, "INICIADO");
    int _count = 0;
    int state_gestor =0;
    msg_queue_t *msg = NULL;
    esp_err_t err = ESP_FAIL;
    cJSON *payload = NULL;
    while(1)
    {   
        //ESP_ERROR_CHECK( heap_trace_start(HEAP_TRACE_LEAKS) );
        if(msg != NULL)
        {
            vPortFree(msg);
            
        }
        msg = (msg_queue_t*)pvPortMalloc(sizeof(msg_queue_t));
        ESP_MALLOC_CHECK(msg);
        err = queue_receive(MASTER,100/portTICK_PERIOD_MS,msg);
        if(err == ESP_OK)
        {
            if(msg->dest == MQTT_TX)
            {
                EventBits_t flags = xEventGroupGetBits(Bits_events);
                if(flags & MQTT_CONNECT)
                {
                    err = queue_to_mqtt_publish(msg);
                    
                }
            }
            else if(msg->dest == MQTT_RX || msg->dest == WS_RX)
            {
                    xEventGroupClearBits(Bits_events,MQTT_ON_MESSAGE);
                    led_on_message();
                    payload = cJSON_Parse(msg->msg);
                    if(!cJSON_IsNull(payload))
                    { 
                        if(Find_Key(payload,"dimmer"))
                        {
                            /*payload: { "dimmer": 0 - 100}*/
                            float dimmer = 0.0;
                            ESP_ERROR_CHECK_WITHOUT_ABORT(decode_number_payload(payload,"dimmer",&dimmer));
                            char *buff =(char*)pvPortMalloc(sizeof(float));
                            itoa((int)dimmer,buff,10);
                            //ESP_LOGI(__FUNCTION__,"llegada %f",dimmer);
                            queue_send(DIMMER_RX,buff,"dimmer",100/portTICK_PERIOD_MS);
                            vPortFree(buff);
                           
                        }
                        else if(Find_Key(payload,"temperature"))
                        {
                            /* mandar a dimmer cuando el control de temperatura este echo*/
                        }
                        else if (Find_Key(payload,"storage"))
                        {   /*payload=  {storage:{wifi:{ssid: "",password:"",ip:"",netmask:"",gateway:"",dns1:"",dns2:""},
                            mqtt:{mqtt_host:"", mqtt_uri:"", mqtt_id:"",mqtt_user:"",mqtt_pass:"",mqtt_port: ,mqtt_pub:"",
                             mqtt_sub:""}, pid:{kp:,ki:,kd:,min:,max:}, inverter:{url_inverter:""}}}*/

                            xTaskCreate(&storage_task,"storage_task",3096,payload,1,NULL);
                            //ESP_ERROR_CHECK(task_create(&storage_task,"storage_task",1,payload));
                            
                        }
                        else if( Find_Key(payload,"stream"))
                        {
                            /*payload: {stream:{wifi:{ssid: "",password:"",ip:"",netmask:"",gateway:"",dns1:"",dns2:""},
                            mqtt:{mqtt_host:"", mqtt_uri:"", mqtt_id:"",mqtt_user:"",mqtt_pass:"",mqtt_port: ,mqtt_pub:"",
                            mqtt_sub:""}, pid:{kp:,ki:,kd:,min:,max:}, inverter:{url_inverter:""}}}*/
                            
                            ESP_ERROR_CHECK_WITHOUT_ABORT(stream_pid(payload));
                            
                        }
                        else if(Find_Key(payload,"update"))
                        {
                            
                            /*payload { "update": "http:// ip_server:80/firmware.bin"}*/
                            /* lanzamiento en dir de proyecto: python -m http.server 80 */
                            cJSON *item = cJSON_GetObjectItem(payload,"update");
                            led_Update(); 
                            if(cJSON_IsString(item))
                            {
                                char *url= item->valuestring;
                                xTaskCreate(&Ota_task, "ota_task", 8192, url, 5, NULL);
                                //(task_create(&Ota_task, "ota_task",5,url));
                                
                            }

                        }   
                    }
                   
            }
            else if(msg->dest == DIMMER_TX)
            {
                if( strcmp(msg->topic,"level")== 0)
                {
                    state_gestor =atoi(msg->msg);
                    char *keep = (char*)pvPortMalloc(sizeof(char) * MAX_PAYLOAD);
                    err = Keepalive(state_gestor,keep);
                    if(err == ESP_OK)
                    {
                        mqtt_publish(keep,strlen(keep),"NONE");
                    }
                    vPortFree(keep);
                    ESP_LOGW(__FUNCTION__,"memoria libre %lu",free_mem());

                }
            }
            else if(msg->dest == OLED_TX)
            {
                printf("mensaje %s\n",msg->msg);
            }
            else
            {
                queue_send(msg->dest,msg->msg,msg->topic,100/portTICK_PERIOD_MS);
            }
        }
        else
        {
            vTaskDelay(100/portTICK_PERIOD_MS);
        }
        EventBits_t flags = xEventGroupGetBits(Bits_events);
        if(flags & WIFI_CHANGE)
        {
            ESP_LOGW(__FUNCTION__,"llamada a WIFI_CHANGE: %d",_count);
            xEventGroupClearBits(Bits_events,WIFI_CHANGE);
            if(_count > 6)
            {
                wifi_mode_t actual_mode = WIFI_MODE_NULL;
                _count =0;
                if(esp_wifi_get_mode(&actual_mode) == ESP_OK)
                {
                    if(actual_mode == WIFI_MODE_AP)
                    {   
                        ESP_LOGW(__FUNCTION__,"Desactivando Ap");
                        Wifi_run(WIFI_MODE_STA);
                    }
                    else if(actual_mode == WIFI_MODE_APSTA)
                    {
                        ESP_LOGW(__FUNCTION__,"Activando Ap");
                        Wifi_run(WIFI_MODE_AP);
                    }
                }

            }
            else
            {
                _count++;
            }
            
        }
        
       
    }
    vTaskDelete(NULL);
}


void app_main(void)
{
    
    printf("	      @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");              
    printf("              @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.\n");                
    printf("              @@@@*@@@@@@@@@@@@@@@*@@@@@@*@@@@@@@@@@@@@@@@ \n");               
    printf("               @@@**@@@@**@@**@@@***@@@@***/@@@@*****@@@@@\n");                
    printf("               @@****@&****@***/******@&*****@@*******#@@@\n");                
    printf("                *********@**@**************************** \n");                  
    printf("                *****@&#********************************* \n");            
    printf("                 **@************************************  \n");                    
    printf("                 *****@@@@@@@@@@@@*****@@@@@@@@@@@@*****  \n");                 
    printf("                 ******          *******          ******  \n");                 
    printf("                  ****    ,@@     *****     @@     ****   \n");                  
    printf("              **********    .  **/,*****/.      .********** \n");                
    printf("             ***##*************************************##*** \n");               
    printf("              ********************************************* \n");                
    printf("                   ***********************************    \n");                  
    printf("                   ********  ===============  *****@**     \n");                 
    printf("             @@@@  *******************************@@*,  @@@@\n");                
    printf("             @@@@@@@*************************/&@*****@@@@@@@\n");                
    printf("             @@@@   *****************@@@@@*/*********   @@@@\n");                
    printf("                     *******************************        \n");
    Machine_init();
    vTaskDelay(2000/portTICK_PERIOD_MS);
    Wifi_run(WIFI_MODE_STA);
    printf("versión actual: %f\n",VERSION);

}


