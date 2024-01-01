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
bool Find_Key(cJSON *obj, const char* key)
{
  cJSON *probe = obj->child;
  if (strcmp(probe->string,key) == 0)
  {
    return true;
  }
 
  probe = probe->next;
 
  while(probe != NULL)
  {
    if (strcmp(probe->string,key) == 0)
    {

        return true;
    }
    else
    {
        probe = probe->next;

    }
   
  }
  return false;
}
esp_err_t decode_number_payload(cJSON *payload, char *key, float *exit)
{
    
    cJSON *item = cJSON_GetObjectItem(payload,key);
    if(cJSON_IsNumber(item))
    {   
        
        *(float*)exit =item->valuedouble;
        return ESP_OK;
    }
    return ESP_FAIL;

}
esp_err_t decode_string_payload(cJSON *payload, char *key, char *exit)
{
    cJSON *item = cJSON_GetObjectItem(payload,key);
    if(cJSON_IsString(item))
    {
        strcpy((void*)exit,item->valuestring);
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t decode_payload(char *msg, char * key,void *exit)
{
    
    cJSON *payload = NULL;
    payload = cJSON_Parse(msg);
    if(!Find_Key(payload,key))
    {
        return ESP_FAIL;
    }
    cJSON *item = cJSON_GetObjectItem(payload,key);
    if(cJSON_IsString(item))
    {
        strcpy((void*)exit,item->valuestring);
        return ESP_OK;
    }
    else if(cJSON_IsNumber(item))
    {   
        *(float*)exit = item->valuedouble;
        return ESP_OK;
    }
    cJSON_Delete(payload);
    return ESP_FAIL;
}


