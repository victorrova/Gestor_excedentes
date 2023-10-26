#ifndef STORAGE_H
#define STORAGE_H
#include <stdio.h>
#include <inttypes.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

/* 
____WIFI___
ip = ip
mascara subred = netamask
gateway = gateway
dns1 = dns1
dns2 = dns2
___MQTT____
ip broker = mqtt_host
url brokder = mqtt_uri
id = mqtt_id
username = mqtt_user
password  = mqtt_pass
puerto = mqtt_port
topic publicar = mqtt_pub
topic subscribir = mqtt_sub

*/


void storage_init(void);

esp_err_t storage_load(nvs_type_t type,const char* key,void* data, size_t len);
size_t storage_get_size(const char *key);
esp_err_t storage_save(nvs_type_t type,const char* key,void* data);
esp_err_t storage_erase(void);
void check_nvs(void);
#endif