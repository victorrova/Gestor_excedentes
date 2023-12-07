#ifndef STORAGE_H
#define STORAGE_H
#include <stdio.h>
#include <cJSON.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <inttypes.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "helper.h"
#include "msgqueue.h"
/* 
____WIFI___
ssid
password
ip = ip
mascara subred = netmask
gateway = gateway
dns1 = dns1
dns2 = dns2
___MQTT____
ip broker = mqtt_host
url broker = mqtt_uri
id = mqtt_id
username = mqtt_user
password  = mqtt_pass
puerto = mqtt_port
topic publicar = mqtt_pub
topic subscribir = mqtt_sub
___PID___
ki
kp
kd
pid_max
pid_min
__URL_INVERTER__
url_inverter
*/



union float_converter{
  float    fl;
  uint32_t ui;
};

void storage_init(void);
esp_err_t storage_load(nvs_type_t type,const char* key,void* data, size_t len);
size_t storage_get_size(const char *key);
esp_err_t storage_save(nvs_type_t type,const char* key,void* data);
esp_err_t storage_erase(void);
void check_nvs(void);
esp_err_t storage_erase_key(char *key);
void storage_task(void *Pvparams);
char *storage_get_config(void);
#endif