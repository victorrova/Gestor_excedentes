#include "storage.h"
#include "wifi.h"
#include "mqtt.h"
#include "pid.h"
#include "dimmer.h"
#include "components/hlw8032/include/hlw8032.h"




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
    ESP_ERROR_CHECK(mqtt_init());
    Meter_init();
}

void Com_Task(void *pvparams)
{
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
                        char *buff = malloc(sizeof(float));
                        itoa((int)dimmer,buff,10);
                        queue_send(DIMMER_RX,buff,"dimmer",100);
                        free(buff);

                    }
                    else if(Find_Key(payload,"temperature"))
                    {
                        /* mandar a dimmer cuando el control de temperatura este echo*/
                    }
                    else if (Find_Key(payload,"config"))
                    {
                        /* code */
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

bool _Find_Key(cJSON *obj, const char* key)
{
    /*1 comprobar el primero 
      2 meter en el bucle wile has Null hasta terminar*/
cJSON *car = obj->child;
while(car != NULL)
{
   Find_Key(car,key);
   car = car->next;
   if(cJSON_IsObject(car))
   {
        printf("esbojeto\n");
   }
   printf("vuelta\n");
}
return false;
}

void app_main(void)
{
    
    //Machine_init();
    /*ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"ssid", "CASA"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"password","k3rb3r0s"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_U32,"mqtt_port", (uint32_t)1883));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_host", "192.168.0.100"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_sub", "prueba/prueba"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_pub", "prueba/prueba"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"url_inverter", "http://192.168.1.39/measurements.xml"));*/
    //Wifi_run(WIFI_MODE_STA); 
    //dimmer_init();
    //xTaskCreate(&Com_Task,"task1",10000,NULL,3,NULL);
    const char* prueba = "{\"config\":{\"wifi\":{\"ssid\":\"prueba\",\"password\":\"prueba\"}}}";
    cJSON *_prueba = cJSON_Parse(prueba);
    _Find_Key(_prueba,"wifi");
    cJSON * root = cJSON_Parse(prueba);
    cJSON * deviceData = cJSON_GetObjectItem(root,"config");
    if( deviceData ) {
   cJSON *device = deviceData->child;
   while( device ) {
      printf(" key = %s\n",device->string);
      device = device->next;
   }
}

}

