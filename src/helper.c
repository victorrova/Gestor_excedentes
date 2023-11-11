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
    /*1 comprobar el primero 
      2 meter en el bucle wile has Null hasta terminar*/
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


esp_err_t decode_payload(char *msg, char * key,void *exit)
{
    esp_err_t err;

    cJSON *payload = NULL;
    int excedente = 0;
    payload = cJSON_Parse(msg);
    if(!Find_Key(payload,key))
    {
        return ESP_FAIL;
    }
    cJSON *item = cJSON_GetObjectItem(payload,key);
    if(cJSON_IsString(item))
    {
        strcpy((void*)exit,item);
    }
    else if(cJSON_IsNumber(item))
    {
        *(float*)exit = item->valueint;
    }
    cJSON_Delete(payload);
    return err;
}


