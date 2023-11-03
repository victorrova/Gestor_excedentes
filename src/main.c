#include "storage.h"
#include "wifi.h"
#include "mqtt.h"
#include "pid.h"
#include "dimmer.h"


void app_main(void)
{
    storage_init();
    ESP_ERROR_CHECK(storage_erase());
    ESP_ERROR_CHECK(Event_init());
    ESP_ERROR_CHECK(queue_start());
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"ssid", "CASA"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"password","k3rb3r0s"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_U32,"mqtt_port", (uint32_t)1883));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_host", "192.168.0.100"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_sub", "prueba/prueba"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_pub", "prueba/prueba"));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"url_inverter", "http://192.168.1.39/measurements.xml"));
    Wifi_init();
    ESP_ERROR_CHECK(mqtt_init());
    Wifi_run(WIFI_MODE_STA); 
    vTaskDelay(5000/portTICK_PERIOD_MS);
    mqtt_publish("hola mundo!");
    vTaskDelay(5000/portTICK_PERIOD_MS);
    mqtt_publish("hola mund1o!");
    TaskHandle_t dimmer = NULL;
    dimmer_init(dimmer);
    
}

