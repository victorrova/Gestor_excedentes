#include "storage.h"


static nvs_handle_t store_handle;


esp_err_t storage_init(void)
{
    esp_err_t err = ESP_FAIL;
    err = nvs_flash_init();
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
    return err;
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
    err = nvs_get_str(store_handle,key,NULL,&len);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND)
    {   
        ESP_LOGE(__FUNCTION__,"[ERROR] %s",esp_err_to_name(err));
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
     nvs_entry_info(it, &info); 
     printf("key '%s', type '%d' \n", info.key, info.type);
     res = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);
}
void storage_get_nvs_size(void)
{
    nvs_stats_t stat;
    nvs_get_stats(NULL, &stat);
    printf("Count: UsedEntries = (%d), FreeEntries = (%d), namespaces = (%d), AllEntries = (%d)\n",
       stat.used_entries, stat.free_entries, stat.namespace_count, stat.total_entries);
}
char *storage_get_config(void)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *storage = cJSON_CreateObject();
    cJSON *wifi = cJSON_CreateObject();
    cJSON *mqtt = cJSON_CreateObject();
    cJSON *pid = cJSON_CreateObject();
    cJSON *inverter = cJSON_CreateObject();

    size_t ssid_len = storage_get_size("ssid");
    if(ssid_len > 0)
    {
        char* ssid = (char*)malloc(sizeof(char) * ssid_len);
        ESP_MALLOC_CHECK(ssid);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"ssid",ssid,&ssid_len));
        cJSON_AddStringToObject(wifi,"ssid",ssid);
        free(ssid);
    }

    size_t pass_len = storage_get_size("password");
    if(pass_len >0)
    {
        char* pass = (char*)malloc(sizeof(char) * pass_len);
        ESP_MALLOC_CHECK(pass);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"password",pass,&pass_len));
        cJSON_AddStringToObject(wifi,"password",pass);
        free(pass);
    }
    size_t ip_len = storage_get_size("ip");
    if(ip_len > 0)
    {
        char* ip = (char*)malloc(sizeof(char) * ip_len);
        ESP_MALLOC_CHECK(ip);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"ip",ip,&ip_len));
        cJSON_AddStringToObject(wifi,"ip",ip);
        free(ip);
    }
    size_t netmask_len = storage_get_size("netmask");
    if(netmask_len > 0)
    {
        char* netmask = (char*)malloc(sizeof(char) * netmask_len);
        ESP_MALLOC_CHECK(netmask);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"netmask",netmask,&netmask_len));
        cJSON_AddStringToObject(wifi,"netmask",netmask);
        free(netmask);
    }
    size_t gateway_len = storage_get_size("gateway");
    if(gateway_len > 0)
    {
        char* gateway = (char*)malloc(sizeof(char) * gateway_len);
        ESP_MALLOC_CHECK(gateway);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"gateway",gateway,&gateway_len));
        cJSON_AddStringToObject(wifi,"gateway",gateway);
        free(gateway);
    }
    size_t dns1_len = storage_get_size("dns1");
    if(dns1_len > 0)
    {
        char* dns1 = (char*)malloc(sizeof(char) * dns1_len);
        ESP_MALLOC_CHECK(dns1);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"dns1",dns1,&dns1_len));
        cJSON_AddStringToObject(wifi,"dns1",dns1);
        free(dns1);
    }
    size_t dns2_len = storage_get_size("dns2");
    if(dns2_len > 0)
    {
        char* dns2 = (char*)malloc(sizeof(char) * dns2_len);
        ESP_MALLOC_CHECK(dns2);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"dns2",dns2,&dns2_len));
        cJSON_AddStringToObject(wifi,"dns2",dns2);
        free(dns2);
    }
    cJSON_AddItemToObject(storage,"wifi",wifi);

    size_t mqtt_host_len = storage_get_size("mqtt_host");
    if(mqtt_host_len >0)
    {
        char *mqtt_host = (char*)malloc(sizeof(char)*mqtt_host_len);
        ESP_MALLOC_CHECK(mqtt_host);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"mqtt_host",mqtt_host,&mqtt_host_len));
        cJSON_AddStringToObject(mqtt,"mqtt_host",mqtt_host);
        free(mqtt_host);
    }
    size_t mqtt_uri_len = storage_get_size("mqtt_uri");
    if(mqtt_uri_len >0)
    {
        char *mqtt_uri = (char*)malloc(sizeof(char)*mqtt_uri_len);
        ESP_MALLOC_CHECK(mqtt_uri);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"mqtt_uri",mqtt_uri,&mqtt_uri_len));
        cJSON_AddStringToObject(mqtt,"mqtt_uri",mqtt_uri);
        free(mqtt_uri);
    }
    size_t mqtt_id_len = storage_get_size("mqtt_id");
    if(mqtt_id_len >0)
    {
        char *mqtt_id = (char*)malloc(sizeof(char)*mqtt_id_len);
        ESP_MALLOC_CHECK(mqtt_id);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"mqtt_id",mqtt_id,&mqtt_id_len));
        cJSON_AddStringToObject(mqtt,"mqtt_id",mqtt_id);
        free(mqtt_id);
    }
    size_t mqtt_user_len = storage_get_size("mqtt_user");
    if(mqtt_user_len >0)
    {
        char *mqtt_user = (char*)malloc(sizeof(char)*mqtt_user_len);
        ESP_MALLOC_CHECK(mqtt_user);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"mqtt_user",mqtt_user,&mqtt_user_len));
        cJSON_AddStringToObject(mqtt,"mqtt_user",mqtt_user);
        free(mqtt_user);
    }
    size_t mqtt_pass_len = storage_get_size("mqtt_pass");
    if(mqtt_pass_len >0)
    {
        char *mqtt_pass = (char*)malloc(sizeof(char)*mqtt_pass_len);
        ESP_MALLOC_CHECK(mqtt_pass);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"mqtt_pass",mqtt_pass,&mqtt_pass_len));
        cJSON_AddStringToObject(mqtt,"mqtt_pass",mqtt_pass);
        free(mqtt_pass);
    }
    size_t mqtt_pub_len = storage_get_size("mqtt_pub");
    if(mqtt_pub_len >0)
    {
        char *mqtt_pub = (char*)malloc(sizeof(char)*mqtt_pub_len);
        ESP_MALLOC_CHECK(mqtt_pub);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"mqtt_pub",mqtt_pub,&mqtt_pub_len));
        cJSON_AddStringToObject(mqtt,"mqtt_pub",mqtt_pub);
        free(mqtt_pub);
    }
    size_t mqtt_sub_len = storage_get_size("mqtt_sub");
    if(mqtt_sub_len >0)
    {
        char *mqtt_sub = (char*)malloc(sizeof(char)*mqtt_sub_len);
        ESP_MALLOC_CHECK(mqtt_sub);
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_STR,"mqtt_sub",mqtt_sub,&mqtt_sub_len));
        cJSON_AddStringToObject(mqtt,"mqtt_sub",mqtt_sub);
        free(mqtt_sub);
    }
    size_t port_len = storage_get_size("mqtt_port");
    if(port_len >0)
    {
        uint32_t port = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,"mqtt_port",port,NULL));
        
        cJSON_AddNumberToObject(pid,"mqtt_port",port);
    }
    cJSON_AddItemToObject(storage,"mqtt",mqtt);

    union float_converter converter;
    
    size_t kp_len = storage_get_size("kp");
    if(kp_len >0)
    {
        uint32_t kp = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,"kp",kp,NULL));
        
        converter.ui = kp;
        cJSON_AddNumberToObject(pid,"kp",converter.fl);
    }
    size_t ki_len = storage_get_size("ki");
    if( ki_len > 0)
    {
        uint32_t ki = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,"ki",ki,NULL));

        converter.ui = ki;
        cJSON_AddNumberToObject(pid,"ki",converter.fl);
    }
    size_t kd_len = storage_get_size("kd");
    if( kd_len > 0)
    {
        uint32_t kd = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,"kd",kd,NULL));
        converter.ui = kd;
        cJSON_AddNumberToObject(pid,"kd",converter.fl);
    }
    size_t pid_min_len = storage_get_size("pid_min");
    if(pid_min_len > 0)
    {
        uint32_t pid_min = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,"pid_min",pid_min,NULL));
        cJSON_AddNumberToObject(pid,"min",1-pid_min);
    }
    size_t pid_max_len = storage_get_size("pid_max");
    if(pid_max_len > 0)
    {
        uint32_t pid_max = 0;
        ESP_ERROR_CHECK_WITHOUT_ABORT(storage_load(NVS_TYPE_U32,"pid_max",pid_max,NULL));
        cJSON_AddNumberToObject(pid,"max",pid_max);
    }
    cJSON_AddItemToObject(storage,"pid",pid);
    size_t url_len = storage_get_size("url_inverter");
    if(url_len >0)
    {
        char *url = (char*)malloc(sizeof(char) * url_len);
        ESP_MALLOC_CHECK(url);
        ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"url_inverter",url,&url_len));
        cJSON_AddStringToObject(inverter,"url_inverter",url);
        free(url);
    }
    cJSON_AddItemToObject(storage,"inverter",inverter);
    cJSON_AddItemToObject(root,"storage",storage);
    char *print = cJSON_Print(root);
    cJSON_Delete(root);
    return print;
}

static esp_err_t json_to_nvs(cJSON *json,nvs_type_t type,char *key)
{   
    cJSON *data = cJSON_GetObjectItem(json,key);
    esp_err_t err = ESP_FAIL;
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
    cJSON *config = cJSON_GetObjectItem(msg,"storage");
    if(cJSON_IsNull(config))
    {
        ESP_LOGE(__FUNCTION__,"Json no config");
        vTaskDelete(NULL);
        cJSON_Delete(config);
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
            else if(Find_Key(mqtt,"mqtt_user"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(mqtt,NVS_TYPE_STR,"mqtt_user"));
                ESP_LOGI(__FILE__,"mqtt user saved!");
            }
            else if(Find_Key(mqtt,"mqtt_pass"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(mqtt,NVS_TYPE_STR,"mqtt_pass"));
                ESP_LOGI(__FILE__,"mqtt password saved!");
            }
            else if(Find_Key(mqtt,"mqtt_port"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(mqtt,NVS_TYPE_U32,"mqtt_port"));
                ESP_LOGI(__FILE__,"mqtt port saved!");
            }
            else if(Find_Key(mqtt,"mqtt_pub"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(mqtt,NVS_TYPE_STR,"mqtt_pub"));
                ESP_LOGI(__FILE__,"mqtt publish topic saved!");
            }
            else if(Find_Key(mqtt,"mqtt_sub"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(mqtt,NVS_TYPE_STR,"mqtt_sub"));
                ESP_LOGI(__FILE__,"mqtt subscribe topic saved!");
            }

        }
    }
    else if(Find_Key(config,"pid"))
    {
        cJSON *pid = cJSON_GetObjectItemCaseSensitive(config,"pid");
        if(!cJSON_IsNull(pid))
        {
            if(Find_Key(pid,"kp"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(pid,NVS_TYPE_ANY,"kp"));
                ESP_LOGI(__FILE__,"pid kp saved!");
            }
            else if(Find_Key(pid,"ki"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(pid,NVS_TYPE_ANY,"ki"));
                ESP_LOGI(__FILE__,"pid ki saved!");
            }
            else if(Find_Key(pid,"kd"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(pid,NVS_TYPE_ANY,"kd"));
                ESP_LOGI(__FILE__,"pid kd saved!");
            }
            else if(Find_Key(pid,"min"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(pid,NVS_TYPE_ANY,"min"));
                ESP_LOGI(__FILE__,"pid min saved!");
            }
            else if(Find_Key(pid,"max"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(pid,NVS_TYPE_ANY,"max"));
                ESP_LOGI(__FILE__,"pid max saved!");
            }
        }
    }
    else if(Find_Key(config,"inverter"))
    {
        cJSON *inverter = cJSON_GetObjectItemCaseSensitive(config,"inverter");
        if(!cJSON_IsNull(inverter))
        {
            if(Find_Key(inverter,"url_inverter"))
            {
                ESP_ERROR_CHECK_WITHOUT_ABORT(json_to_nvs(inverter,NVS_TYPE_STR,"url_inverter"));
                ESP_LOGI(__FILE__,"url inverter saved!");

            }   
        }
    }
    cJSON_Delete(config);
    vTaskDelete(NULL);
   
}   