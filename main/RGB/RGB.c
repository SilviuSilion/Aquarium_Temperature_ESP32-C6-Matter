/**
 * @file RGB.c
 * @brief WS2812 RGB LED driver for ESP32-C6
 * 
 * GPIO8 - Single WS2812 LED (GRB order)
 */

#include "RGB.h"

static led_strip_handle_t led_strip;

void RGB_Init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,  // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
}

void Set_RGB(uint8_t red_val, uint8_t green_val, uint8_t blue_val)
{
    // WS2812 on this board uses GRB order - swap R and G
    led_strip_set_pixel(led_strip, 0, green_val, red_val, blue_val);
    led_strip_refresh(led_strip);
}
