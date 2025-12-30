/**
 * @file aquarium_controller.h
 * @brief Aquarium Temperature Controller for ESP32-C6-LCD-1.47
 * 
 * Features:
 * - DS18B20 temperature sensor on GPIO3
 * - RGB LED control based on temperature (30% power)
 * - Matter/HomeKit integration
 * - 5 second update interval
 */

#ifndef AQUARIUM_CONTROLLER_H
#define AQUARIUM_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

// Temperature thresholds
#define TEMP_MIN_NORMAL 23.0f  // Below this: RED LED
#define TEMP_MAX_NORMAL 28.0f  // Above this: RED LED

// DS18B20 configuration
#define DS18B20_GPIO GPIO_NUM_3

// Update interval (milliseconds)
#define TEMP_UPDATE_INTERVAL_MS 5000

// LED brightness (0.0 - 1.0)
#define LED_BRIGHTNESS 0.30f  // 30% power

// Function prototypes
void aquarium_controller_init(void);
void aquarium_start(void);

// Get last valid temperature (for UI)
float aquarium_get_last_temperature(void);
bool aquarium_is_temperature_valid(void);

#endif // AQUARIUM_CONTROLLER_H
