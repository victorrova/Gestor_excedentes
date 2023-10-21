#include "storage.h"
#include "wifi.h"


void app_main(void)
{
    storage_init();
    ESP_ERROR_CHECK(Event_init());
    Wifi_init();
    storage_save(NVS_TYPE_STR,"ssid", "CASA");
    storage_save(NVS_TYPE_STR,"password","k3rb3r0s");
    Wifi_run(WIFI_MODE_AP);
    vTaskDelay(10000/portTICK_PERIOD_MS);
    Wifi_run(WIFI_MODE_STA);

}