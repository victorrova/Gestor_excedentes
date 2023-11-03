#include "helper.h"




int map(int val,int in_min,int in_max,int out_min,int out_max){
    int exit = (val - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    if(exit< out_min){
        return out_min;
    }
    if(exit > out_max){
        return out_max;

    }
    return exit;
}


int decode_payload(char *msg)
{
    cJSON *payload = NULL;
    int excedente = 0;
    payload = cJSON_Parse(msg);
    excedente = cJSON_GetObjectItem(payload,"link_voltage")->valueint;
    cJSON_Delete(payload);
    return excedente;
}

