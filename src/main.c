#include "storage.h"
#include "wifi.h"


void app_main(void)
{
    char* scanner = malloc(sizeof(char)*1024);
    Wifi_start();
    scanner = wifi_scan();
    printf("scanner: %s",scanner);
    free(scanner);
}