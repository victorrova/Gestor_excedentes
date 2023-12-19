#include "event.h"




EventGroupHandle_t Bits_events;
StaticEventGroup_t xCreatedEventGroup;

ESP_EVENT_DEFINE_BASE(MACHINE_EVENTS);

esp_err_t Event_init(void)
{
    
    Bits_events = xEventGroupCreateStatic( &xCreatedEventGroup);
    if(Bits_events == NULL)
    {
        return ESP_FAIL;
    }
    return esp_event_loop_create_default();
}

