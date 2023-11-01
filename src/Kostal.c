
#include "Kostal.h"




static void xml_handler(xr_type_t type, const xr_str_t* name, const xr_str_t* val, void* user_data)
{   
    static char value[24] = "";
    static char unit[24] = "";
    static char Type[24] = "";
    
    if(type == xr_type_attribute )
    {
        char *out =(char*)pvPortMalloc(sizeof(char) * 32);
        bzero(out,32);
        strncpy(out,name->cstr,name->len);
        //out[name->len] = '\0';
        
        if(strcmp(out,"Value") == 0)
        {
            bzero(value,24);
            strncpy(value,val->cstr,val->len);
            
            //value[val->len] ='\0';
            //printf("Value= %s\n",value);
        }
        else if(strcmp(out,"Unit") == 0)
        {
            bzero(unit,24);
            strncpy(unit,val->cstr,val->len);
            //unit[val->len] ='\0';
            //printf("Unit = %s\n",unit);
        }
        else if(strcmp(out,"Type") == 0)
        {
            bzero(Type,24);
            strncpy(Type,val->cstr,val->len);
            //Type[val->len] ='\0';
            //printf("Type = %s\n",Type);
        }
        if(strcmp(Type,"GridPower") == 0)
        {
            *(float*)user_data = (float)atof(value);
            //Type[5] ='\0';
            bzero(Type,strlen(Type));


        }
        vPortFree(out);
    }
 
}

esp_http_client_handle_t  http_begin(const char *url)
{
    esp_http_client_config_t config = {
        .url = url,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_method(client, HTTP_METHOD_GET);
    return client;
}
float Kostal_requests(esp_http_client_handle_t client)
{
    int content_length = 0;
    float a =0.0;
    char *output_buffer = (char*)pvPortMalloc(sizeof(char) * 1800);
    if(output_buffer == NULL)
    {
        ESP_LOGE(__FUNCTION__, "Failed to allocate memory for output buffer");
    }
    bzero(output_buffer,1800);
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) 
    {
        ESP_LOGE(__FUNCTION__, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        vPortFree(output_buffer);
        esp_http_client_close(client);
        
    } 
    else 
    {
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) 
        {
            ESP_LOGE(__FUNCTION__, "HTTP client fetch headers failed");
            vPortFree(output_buffer);
            esp_http_client_close(client);

        } 
        else 
        {
            int data_read = esp_http_client_read_response(client, output_buffer, 1800);
            if (data_read >= 0 && esp_http_client_get_status_code(client) == 200) 
            {
                char *_buffer= (char*)pvPortMalloc(sizeof(char) * data_read);
                bzero(_buffer,data_read);
                size_t m = strlen(output_buffer);
                int j = 0;
                for(int i = 38; i< m;i++)
                {
                    _buffer[j] = output_buffer[i];
                    j++;
                }
                vPortFree(output_buffer);
                
                xr_read(&xml_handler,_buffer,&a);
                esp_http_client_close(client);
                if(_buffer != NULL)
                {
                    vPortFree(_buffer);
                }
            } 
            else 
            {
                ESP_LOGE(__FUNCTION__, "Failed to read response");
                vPortFree(output_buffer);
                esp_http_client_close(client);
            }
        }
    }
    return a;
   
}
