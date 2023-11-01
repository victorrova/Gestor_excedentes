#include "machine.h"








int _free_mem(void)
{
    int a =  esp_get_free_heap_size()/1024;
    ESP_LOGW(__FUNCTION__, "[APP] Free memory: %d Kbytes", a);
    return a;  
}
