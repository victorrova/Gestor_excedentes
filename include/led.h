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

typedef struct color{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    
}color_led_t;

typedef struct led{
    color_led_t color;
    uint8_t pixel[3];
    rmt_channel_handle_t led_chan;
    rmt_encoder_handle_t led_encoder;
    rmt_transmit_config_t tx_config;
}led_params_t;

void led_machine_ok(void);
void led_total_connect(void);
void led_on_message(void);
void led_fail(void);
esp_err_t led_init(void);
void led_off(void);
#endif