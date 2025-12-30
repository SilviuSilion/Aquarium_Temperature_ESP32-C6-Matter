/**
 * @file RGB.h
 * @brief WS2812 RGB LED driver
 */

#ifndef __RGB_H
#define __RGB_H

#include "led_strip.h"

#define BLINK_GPIO 8

void RGB_Init(void);
void Set_RGB(uint8_t red_val, uint8_t green_val, uint8_t blue_val);

#endif
