#ifndef HELPER_H
#define HELPER_H
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "cJSON.h"

#define ESP_MALLOC_CHECK(x)  assert((x) != NULL)

/* copia de arduino map()*/

int map(int val,int in_min,int in_max,int out_min,int out_max);

/*busca una clave en un objeto json*/

bool Find_Key(cJSON *obj, const char* key);

/*devuelve el valor numérico de la clave dada */

esp_err_t decode_number_payload(cJSON *payload, char *key, float *exit);

/*devuelve el valor cadena de la clave dada */

esp_err_t decode_string_payload(cJSON *payload, char *key, char *exit);

/*devuelve le valor de la clave dada */


esp_err_t decode_payload(char *msg, char * key,void *exit);
#endif
