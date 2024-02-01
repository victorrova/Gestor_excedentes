#include <stdio.h>
#include "ring_queue.h"
#include "config.h"
#include "esp_system.h"

void app_main(void)
{

    printf("	      @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");              
    printf("              @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@.\n");                
    printf("              @@@@*@@@@@@@@@@@@@@@*@@@@@@*@@@@@@@@@@@@@@@@ \n");               
    printf("               @@@**@@@@**@@**@@@***@@@@***/@@@@*****@@@@@\n");                
    printf("               @@****@&****@***/******@&*****@@*******#@@@\n");                
    printf("                *********@**@**************************** \n");                  
    printf("                *****@&#********************************* \n");            
    printf("                 **@************************************  \n");                    
    printf("                 *****@@@@@@@@@@@@*****@@@@@@@@@@@@*****  \n");                 
    printf("                 ******          *******          ******  \n");                 
    printf("                  ****    ,@@     *****     @@     ****   \n");                  
    printf("              **********    .  **/,*****/.      .********** \n");                
    printf("             ***##*************************************##*** \n");               
    printf("              ********************************************* \n");                
    printf("                   ***********************************    \n");                  
    printf("                   ********  ===============  *****@**     \n");                 
    printf("             @@@@  *******************************@@*,  @@@@\n");                
    printf("             @@@@@@@*************************/&@*****@@@@@@@\n");                
    printf("             @@@@   *****************@@@@@*/*********   @@@@\n");                
    printf("                     *******************************        \n"); 
    //Machine_init();
    //vTaskDelay(2000/portTICK_PERIOD_MS);
    //Wifi_run(WIFI_MODE_STA);
    printf("versión actual: %f\n",VERSION);
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"ssid", "CASA"));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"password","k3rb3r0s"));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_U32,"mqtt_port", (uint32_t)1883));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_host", "192.168.0.100"));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_sub", "/gestor/envio"));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"mqtt_pub", "/gestor/response"));
    //ESP_ERROR_CHECK(storage_save(NVS_TYPE_STR,"url_inverter", "http://192.168.1.39/measurements.xml"));
    xqueue_start();
    esp_err_t err= ESP_FAIL;
    while(1)
    {

        printf("memoria inicio %lu\n",esp_get_free_heap_size());
        msg_queue_t *msg = (msg_queue_t*)malloc(sizeof(msg_queue_t));
        xqueue_send(MQTT_TX,"hyiadkkdagggagsgffa","NONE",100/portTICK_PERIOD_MS);
        vTaskDelay(500/portTICK_PERIOD_MS);
        err = xqueue_receive(MQTT_TX,portMAX_DELAY,msg);
        if(err ==ESP_OK)
        {
            printf("salida %s\n",msg->msg);
        }
        else
        {
            printf("error\n");
        }
        free(msg);
        printf("memoria final %lu\n",esp_get_free_heap_size());

    }
}



