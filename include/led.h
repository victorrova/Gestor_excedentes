#ifndef LED_H
#define LED_H



#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "led_strip_encoder.h"
#include "event.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define RMT_LED_STRIP_GPIO_NUM      2
#define LED_NUMBERS         1
#define CHASE_SPEED_MS      500

typedef enum{
    MODE_WARNING,
    MODE_OPERATION,
    MODE_LOAD,
    MODE_ERROR,
    MODE_MSG,
}led_mode_t;
typedef struct led{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
}color_led_t;

#endif