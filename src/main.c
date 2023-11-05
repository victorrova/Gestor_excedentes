#include "storage.h"
#include "wifi.h"
#include "mqtt.h"
#include "pid.h"
#include "dimmer.h"
#include "components/hlw8032/include/hlw8032.h"


void Machine_init(void)
{
    storage_init();
    ESP_ERROR_CHECK(Event_init());
    ESP_ERROR_CHECK(queue_start());
    termistor_init();
    Fan_init();
    Wifi_init();
    ESP_ERROR_CHECK(mqtt_init());
}
void Meter_init(void)
{
    hlw8032_t  hlw_meter;
    ESP_ERROR_CHECK(hlw8032_serial_begin(&hlw_meter,2,17,256));
    hlw8032_set_I_coef_from_R(&hlw_meter, 0.001);
    hlw8032_set_V_coef_from_R(&hlw_meter, 1880000, 1000);

}

void pid_temp(void *params)
{
    PID_IncTypeDef pid;
    pid.Kp = 0.5;
    pid.Ki = 0.2;
    pid.Kd = 0.6;
    pid.CumError =0;
    pid.Error = 0;
    pid.LastError = 0;
    pid.max = 5;
    pid.min = -5;
    int temp =0;
    int _pid = 0;

    while(1)
    {
        temp = (int)temp_termistor();
        if(temp > 60)
        {
            Fan_state(1);
        }
        else if(temp > 65)
        {
            _pid = PID(50,temp,&pid);
            queue_send(DIMMER,(const char*)_pid,"temp_pid",100);

        }
        else if(temp < 50)
        {
            Fan_state(0);
        }
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);

}

void Core(void)
{
    
}

void app_main(void)
{
    

    /*ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"ssid", "CASA"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"password","k3rb3r0s"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_U32,"mqtt_port", (uint32_t)1883));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_host", "192.168.0.100"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_sub", "prueba/prueba"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_pub", "prueba/prueba"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"url_inverter", "http://192.168.1.39/measurements.xml"));*/
    Wifi_run(WIFI_MODE_STA); 
    vTaskDelay(5000/portTICK_PERIOD_MS);
    mqtt_publish("hola mundo!");
    vTaskDelay(5000/portTICK_PERIOD_MS);
    mqtt_publish("hola mund1o!");
    dimmer_init();
    
}

