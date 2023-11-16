#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "cJSON.h"

#define ESP_MALLOC_CHECK(x)  assert((x) != NULL)
int map(int val,int in_min,int in_max,int out_min,int out_max);
esp_err_t decode_payload(char *msg, char * key,void *exit);
#endif
