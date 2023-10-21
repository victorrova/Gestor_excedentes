#ifndef STORAGE_H
#define STORAGE_H
#include <stdio.h>
#include <inttypes.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"




void storage_init(void);

esp_err_t storage_load(nvs_type_t type,const char* key,void* data, size_t len);
size_t storage_get_size(const char *key);
esp_err_t storage_save(nvs_type_t type,const char* key,void* data);
esp_err_t storage_erase(void);

#endif