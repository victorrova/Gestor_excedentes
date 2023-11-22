#include "storage.h"


static nvs_handle_t store_handle;

esp_err_t json_to_nvs(cJSON *json)
{   
    cJSON *obj = NULL;
    cJSON *cal = json->child;
    while(cal != NULL)
    {
        if(cJSON_IsObject(cal))
        {
            obj = cJSON_GetObjectItem(cal,cal->string);
            cal = obj;
        }
        else if(cJSON_IsNumber(cal))
        {
            cal = cal->next;
        }   
        else if(cJSON_IsString(cal))
        {
            cal = cal->next;
        }
 
    }
}
void storage_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    err = nvs_open("storage",NVS_READWRITE,&store_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"Error (%s) opening NVS handle!", esp_err_to_name(err));
    } 
    else 
    {
       ESP_LOGI(__FUNCTION__,"storage system...[OK]");
       nvs_close(store_handle); 
    }
}

esp_err_t storage_load(nvs_type_t type,const char* key,void* data, size_t len)
{

    esp_err_t err;
    err = nvs_open("storage",NVS_READWRITE,&store_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }

    switch (type)
    {
    case  NVS_TYPE_U8:
        err = nvs_get_u8(store_handle,key,(uint8_t)data);
        break;
    case NVS_TYPE_U16:
        err = nvs_get_u16(store_handle,key,(uint16_t)data);
        break;
    case NVS_TYPE_U32:
        err = nvs_get_u32(store_handle,key,(uint32_t)data);
        break;
    case NVS_TYPE_STR:
        err = nvs_get_str(store_handle,key,(char*)data, &len);
    default:
        break;
    }
    nvs_close(store_handle);
    return err;
}
size_t storage_get_size(const char *key)
{
    size_t len = 0;
    esp_err_t err;
    err = nvs_open("storage",NVS_READWRITE,&store_handle);
    esp_err_t rr = nvs_get_str(store_handle,key,NULL,&len);
    if (rr != ESP_OK && rr != ESP_ERR_NVS_NOT_FOUND)
    {   
        ESP_LOGE(__FUNCTION__,"error %s",esp_err_to_name(rr));
        return 0;
    }
    return len;
}
esp_err_t storage_save(nvs_type_t type,const char* key,void* data)
{
    esp_err_t err;
    err = nvs_open("storage",NVS_READWRITE,&store_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }
     
    switch (type)
    {
    case  NVS_TYPE_U8:
        err = nvs_set_u8(store_handle,key,(uint8_t)data);
        break;
    case NVS_TYPE_U16:
        err = nvs_set_u16(store_handle,key,(uint16_t)data);
        break;
    case NVS_TYPE_U32:
        err = nvs_set_u32(store_handle,key,(uint32_t)data);
        break;
    case NVS_TYPE_STR:
        err = nvs_set_str(store_handle,key,(const char*)data);
    default:
        break;
    }

    switch (err) {
        case ESP_OK:
            err = nvs_commit(store_handle);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGE(__FUNCTION__,"Key-value not found!");
            break;
        default :
            ESP_LOGE(__FUNCTION__,"Error (%s) reading!", esp_err_to_name(err));
        }
    nvs_close(store_handle);
    return err;
}

esp_err_t storage_erase(void)
{
    esp_err_t err;
    err = nvs_open("storage",NVS_READWRITE,&store_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }
    err =  nvs_erase_all(store_handle);
    nvs_close(store_handle);
    return err;
}
esp_err_t storage_erase_key(char *key)
{
    esp_err_t err;
    err = nvs_open("storage",NVS_READWRITE,&store_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(__FUNCTION__,"Error (%s) opening NVS handle!", esp_err_to_name(err));
        return err;
    }
    err =  nvs_erase_key(store_handle,key);
    nvs_close(store_handle);
    return err;
}

void check_nvs(void)
{
    nvs_iterator_t it = NULL;
    esp_err_t res = nvs_entry_find("nvs", "storage", NVS_TYPE_ANY, &it);
    while(res == ESP_OK) 
    {
     nvs_entry_info_t info;
     nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
     printf("key '%s', type '%d' \n", info.key, info.type);
     res = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);
}

void storage_task(void *Pvparams)
{
    cJSON *msg = (cJSON*)Pvparams;
    cJSON *config = cJSON_GetObjectItem(msg,"config");
    if(cJSON_IsNull(config))
    {
        ESP_LOGE(__FUNCTION__,"Json no config");
        vTaskDelete(NULL);  
    }
    if(Find_Key(config,"wifi"))
    {
        cJSON *wifi = cJSON_GetObjectItemCaseSensitive(config,"wifi");
        if(!cJSON_IsNull(wifi))
        {
            if(Find_Key(wifi,"ssid"))
            {
                cJSON *ssid = cJSON_GetObjectItem(wifi,"ssid");
                if(!cJSON_IsNull(ssid))
                {
                    char *_ssid = ssid->valuestring;
                    if(storage_get_size("ssid") > 0)
                    {
                        storage_erase_key("ssid");  
                    }
                    storage_save(NVS_TYPE_STR,"ssid",_ssid);
                }
            }
            else if(Find_Key(wifi,"password"))
            {
                cJSON *password = cJSON_GetObjectItem(wifi,"password");
                if(!cJSON_IsNull(password))
                {
                    char *_pwd = password->valuestring;
                    if(storage_get_size("password") > 0)
                    {
                        storage_erase_key("password");   
                    }
                    storage_save(NVS_TYPE_STR,"password",_pwd);
                }
            }
            
        }
    }

    
}   