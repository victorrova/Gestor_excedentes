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
       
        msg_queue_t msg = Master_queue_receive(10);
        if(msg.len_msg >0)
        {
            if(msg.dest == MQTT_TX)
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(mqtt_publish(msg.msg,msg.len_msg,NULL));
            }
            else if(msg.dest == MQTT_RX)
            {
                PASS

            }   
        }
    } 
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
    
}

