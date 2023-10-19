#include "storage.h"
#include "wifi.h"


void app_main(void)
{
    storage_init();
 
    storage_save(NVS_TYPE_STR,"ssid", "CASA");
    storage_save(NVS_TYPE_STR,"password","k3rbr0s");
    //char* scanner = malloc(sizeof(char)*1024);
    Wifi_start();
    //scanner = wifi_scan();
    //printf("scanner: %s",scanner);
    //free(scanner);
}