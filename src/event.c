#include "event.h"




esp_err_t Event_init(void)
{
    Bits_events = xEventGroupCreateStatic( &xCreatedEventGroup);
    if(Bits_events == NULL)
    {
        return ESP_FAIL;
    }
    return esp_event_loop_create_default();
}

