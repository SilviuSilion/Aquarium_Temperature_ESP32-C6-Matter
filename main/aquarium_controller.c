/**
 * @file aquarium_controller.c
 * @brief Aquarium Temperature Controller
 * 
 * - DS18B20 temperature reading every 5 seconds
 * - RGB LED at 30% brightness
 * - Matter/HomeKit updates
 * - Retry on CRC errors
 */

#include "aquarium_controller.h"
#include "RGB.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <math.h>
#include "Matter/aquarium_matter.h"

static const char *TAG = "AQUARIUM";

// Global temperature storage
static float g_last_temperature = NAN;
static bool g_temperature_valid = false;

// Max retries for reading
#define MAX_READ_RETRIES 3

// ============================================================================
// DS18B20 1-Wire Implementation
// ============================================================================

static inline void ow_delay_us(uint32_t us) { 
    esp_rom_delay_us(us); 
}

static void ow_drive_low(gpio_num_t pin) {
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 0);
}

static void ow_release(gpio_num_t pin) {
    gpio_set_direction(pin, GPIO_MODE_INPUT_OUTPUT_OD);
    gpio_set_level(pin, 1);
}

static int ow_read(gpio_num_t pin) { 
    return gpio_get_level(pin); 
}

static int ow_reset(gpio_num_t pin) {
    ow_drive_low(pin);
    ow_delay_us(480);
    ow_release(pin);
    ow_delay_us(70);
    int presence = (ow_read(pin) == 0);
    ow_delay_us(410);
    return presence ? 0 : -1;
}

static void ow_write_bit(gpio_num_t pin, int v) {
    if (v) {
        ow_drive_low(pin);
        ow_delay_us(6);
        ow_release(pin);
        ow_delay_us(64);
    } else {
        ow_drive_low(pin);
        ow_delay_us(60);
        ow_release(pin);
        ow_delay_us(10);
    }
}

static int ow_read_bit(gpio_num_t pin) {
    ow_drive_low(pin);
    ow_delay_us(6);
    ow_release(pin);
    ow_delay_us(9);
    int b = ow_read(pin);
    ow_delay_us(55);
    return b;
}

static void ow_write_byte(gpio_num_t pin, uint8_t v) {
    for (int i = 0; i < 8; i++) 
        ow_write_bit(pin, (v >> i) & 1);
}

static uint8_t ow_read_byte(gpio_num_t pin) {
    uint8_t r = 0;
    for (int i = 0; i < 8; i++) 
        r |= (ow_read_bit(pin) << i);
    return r;
}

// ============================================================================
// CRC8 for DS18B20
// ============================================================================

static uint8_t crc8(uint8_t *data, uint8_t len) {
    uint8_t crc = 0;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t byte = data[i];
        for (uint8_t j = 0; j < 8; j++) {
            uint8_t mix = (crc ^ byte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            byte >>= 1;
        }
    }
    return crc;
}

// ============================================================================
// Single Temperature Read Attempt
// ============================================================================

static float try_read_temperature(void) {
    gpio_num_t pin = DS18B20_GPIO;
    
    // Reset and check presence
    if (ow_reset(pin) != 0) {
        return NAN;
    }
    
    // Skip ROM + Convert Temperature
    ow_write_byte(pin, 0xCC);
    ow_write_byte(pin, 0x44);
    
    // Wait for conversion
    vTaskDelay(pdMS_TO_TICKS(750));
    
    // Reset again
    if (ow_reset(pin) != 0) {
        return NAN;
    }
    
    // Read scratchpad
    ow_write_byte(pin, 0xCC);
    ow_write_byte(pin, 0xBE);
    
    uint8_t scratchpad[9];
    for (int i = 0; i < 9; i++) {
        scratchpad[i] = ow_read_byte(pin);
    }
    
    // CRC check
    if (crc8(scratchpad, 8) != scratchpad[8]) {
        return NAN;
    }
    
    int16_t raw = (int16_t)((scratchpad[1] << 8) | scratchpad[0]);
    
    // Invalid readings
    if (raw == 0 || raw == 0x0550 || raw == 0xFFFF) {
        return NAN;
    }
    
    float celsius = (float)raw / 16.0f;
    
    // Sanity check
    if (celsius < 5.0f || celsius > 50.0f) {
        return NAN;
    }
    
    return celsius;
}

// ============================================================================
// Read with Retry
// ============================================================================

static float read_ds18b20_temperature(void) {
    for (int retry = 0; retry < MAX_READ_RETRIES; retry++) {
        float temp = try_read_temperature();
        if (!isnan(temp)) {
            return temp;
        }
        if (retry < MAX_READ_RETRIES - 1) {
            vTaskDelay(pdMS_TO_TICKS(100));  // Small delay before retry
        }
    }
    return NAN;
}

// ============================================================================
// Public API
// ============================================================================

float aquarium_get_last_temperature(void) {
    return g_last_temperature;
}

bool aquarium_is_temperature_valid(void) {
    return g_temperature_valid;
}

// ============================================================================
// RGB LED Control (30%)
// ============================================================================

static void update_led(float temp) {
    if (isnan(temp) || !g_temperature_valid) {
        Set_RGB((uint8_t)(80 * LED_BRIGHTNESS), 0, (uint8_t)(80 * LED_BRIGHTNESS));
        return;
    }
    
    if (temp < TEMP_MIN_NORMAL || temp > TEMP_MAX_NORMAL) {
        Set_RGB((uint8_t)(255 * LED_BRIGHTNESS), 0, 0);
    } else {
        float progress = (temp - TEMP_MIN_NORMAL) / (TEMP_MAX_NORMAL - TEMP_MIN_NORMAL);
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;
        
        uint8_t g, b;
        if (progress < 0.5f) {
            float t = progress * 2.0f;
            g = (uint8_t)(80 + (180 - 80) * t);
            b = (uint8_t)(180 + (150 - 180) * t);
        } else {
            float t = (progress - 0.5f) * 2.0f;
            g = (uint8_t)(180 + (220 - 180) * t);
            b = (uint8_t)(150 + (80 - 150) * t);
        }
        Set_RGB(0, (uint8_t)(g * LED_BRIGHTNESS), (uint8_t)(b * LED_BRIGHTNESS));
    }
}

// ============================================================================
// Main Task
// ============================================================================

static void aquarium_task(void *arg) {
    ESP_LOGI(TAG, "Temperature monitoring started");
    ESP_LOGI(TAG, "   Interval: %d sec, Retries: %d", TEMP_UPDATE_INTERVAL_MS / 1000, MAX_READ_RETRIES);
    
    while (1) {
        float temp = read_ds18b20_temperature();
        
        if (!isnan(temp)) {
            g_last_temperature = temp;
            g_temperature_valid = true;
            
            ESP_LOGI(TAG, "🌡️ %.1f°C", temp);
            aquarium_matter_update_temperature(temp);
        } else {
            // Keep last valid temperature, just log warning
            ESP_LOGW(TAG, "⚠️ Read failed (keeping %.1f°C)", g_last_temperature);
        }
        
        update_led(g_last_temperature);
        
        // Wait 5 seconds (precise timing)
        vTaskDelay(pdMS_TO_TICKS(TEMP_UPDATE_INTERVAL_MS));
    }
}

// ============================================================================
// Init
// ============================================================================

void aquarium_controller_init(void) {
    ESP_LOGI(TAG, "Controller init - GPIO%d", DS18B20_GPIO);
    
    // Configure GPIO for DS18B20 (once at init)
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << DS18B20_GPIO,
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
    ow_release(DS18B20_GPIO);
}

void aquarium_start(void) {
    xTaskCreatePinnedToCore(aquarium_task, "aquarium", 4096, NULL, 5, NULL, 0);
}
