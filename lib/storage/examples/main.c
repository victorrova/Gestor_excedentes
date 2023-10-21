#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "storage.h"
void app_main(void)
{
    storage_init();
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_U16,"prueba",26));
    ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"text","MIMONOAMELIO"));
    uint16_t data = 0;
    ESP_ERROR_CHECK(storage_load(NVS_TYPE_U16,"prueba",&data,NULL));
    char *prim = (char*)malloc(25 *sizeof(char));
    ESP_ERROR_CHECK(storage_load(NVS_TYPE_STR,"text",prim,25));
    printf("resultado = %d\n",data);
    printf("estring = %s media = %d\n",prim,strlen(prim));
    free(prim);

}
