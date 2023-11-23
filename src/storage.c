#include "storage.h"


static nvs_handle_t store_handle;

static esp_err_t json_to_nvs( int type,const char* key)
{
    return ESP_OK;
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

esp_err_t json_to_nvs(cJSON *json,nvs_type_t type,char *key)
{   
    cJSON *data = cJSON_GetObjectItem(json,key);
    esp_err_t err;
    if(!cJSON_IsNull(data))
    {
        switch (type)
        {
        case NVS_TYPE_STR:
        {
            char *_data= data->valuestring;
            if(storage_get_size(key) > 0)
            {
                err = storage_erase_key(key);  
            }
            err =  storage_save(NVS_TYPE_STR,key,_data);
            
        }
            break;
        case NVS_TYPE_U16:
        {
            uint16_t  _data  = (uint16_t) data->valuedouble;
                        if(storage_get_size(key) > 0)
            {
                err = storage_erase_key(key);  
            }
            err =  storage_save(NVS_TYPE_U16,key,(uint16_t)_data);
            
        }
            break;
        case  NVS_TYPE_U32:
        {
            uint32_t  _data  = (uint32_t) data->valuedouble;
            if(storage_get_size(key) > 0)
            {
                err = storage_erase_key(key);  
            }
            err =  storage_save(NVS_TYPE_U32,key,(uint32_t)_data);
            
        }
            break;
        case NVS_TYPE_ANY:   // tipo float
        {
            union float_converter change;
            change.fl = data->valuedouble;
            if(storage_get_size(key) > 0)
            {
                err = storage_erase_key(key);  
            }
            err =  storage_save(NVS_TYPE_U32,key,(uint32_t)change.ui);
        }
            break;
        default:
            break;
        }
    }
    else
    {
        err= ESP_FAIL;
    }
    return err;
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
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(wifi,NVS_TYPE_STR,"ssid"));
                ESP_LOGI(__FILE__,"ssid saved!");
            }
            else if(Find_Key(wifi,"password"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(wifi,NVS_TYPE_STR,"password"));
                ESP_LOGI(__FILE__,"wifi password saved!");
            }
            else if(Find_Key(wifi,"ip"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(wifi,NVS_TYPE_STR,"ip"));
                ESP_LOGI(__FILE__,"wifi ip saved!");
            }
            else if(Find_Key(wifi,"netmask"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(wifi,NVS_TYPE_STR,"netmask"));
                ESP_LOGI(__FILE__,"wifi netmask saved!");
            }
            else if(Find_Key(wifi,"gateway"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(wifi,NVS_TYPE_STR,"gateway"));
                ESP_LOGI(__FILE__,"wifi gateway saved!");
            }
            else if(Find_Key(wifi,"dns1"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(wifi,NVS_TYPE_STR,"dns1"));
                 ESP_LOGI(__FILE__,"wifi dns1 saved!");
            }
            else if(Find_Key(wifi,"dns2"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(wifi,NVS_TYPE_STR,"dns2"));
                ESP_LOGI(__FILE__,"wifi dns2 saved!");
            }
        }
    }
    else if(Find_Key(config,"mqtt"))
    {
        cJSON *mqtt = cJSON_GetObjectItemCaseSensitive(config,"mqtt");
        if(!cJSON_IsNull(mqtt))
        {
            if(Find_Key(mqtt,"mqtt_host"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(mqtt,NVS_TYPE_STR,"mqtt_host"));
                ESP_LOGI(__FILE__,"mqtt host saved!");
            }
            else if(Find_Key(mqtt,"mqtt_uri"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(mqtt,NVS_TYPE_STR,"mqtt_uri"));
                ESP_LOGI(__FILE__,"mqtt url saved!");
            }
            else if(Find_Key(mqtt,"mqtt_id"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(mqtt,NVS_TYPE_STR,"mqtt_id"));
                ESP_LOGI(__FILE__,"mqtt id saved!");
            }
        }
    }
}   