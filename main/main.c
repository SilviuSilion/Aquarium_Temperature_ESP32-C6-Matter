/**
 * @file main.c
 * @brief Aquarium Temperature Monitor - ESP32-C6
 * 
 * Features:
 * - DS18B20 temperature sensor (5 sec interval)
 * - RGB LED at 18% brightness
 * - Display at 50% brightness
 * - Matter/HomeKit integration
 * 
 * Manufacturer: Claude&Silviu
 * Model: Senzor Apa
 */

#include "ST7789.h"
#include "RGB.h"
#include "Wireless.h"
#include "aquarium_controller.h"
#include "aquarium_ui.h"
#include "Matter/aquarium_matter.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

// Display brightness (0-100)
#define DISPLAY_BRIGHTNESS 50

void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "🐠 Aquarium Temperature Monitor");
    ESP_LOGI(TAG, "   by Claude&Silviu");
    ESP_LOGI(TAG, "");
    
    // RGB LED (first - no dependencies)
    RGB_Init();
    ESP_LOGI(TAG, "✓ RGB LED (18%% brightness)");
    
    // Display (before WiFi - uses SPI2)
    LCD_Init();
    BK_Light(DISPLAY_BRIGHTNESS);
    ESP_LOGI(TAG, "✓ Display (%d%% brightness)", DISPLAY_BRIGHTNESS);
    
    // LVGL
    LVGL_Init();
    
    // Controller
    aquarium_controller_init();
    
    // UI
    aquarium_ui_init();
    
    // Force first UI render before WiFi/Matter (which take time)
    lv_timer_handler();
    
    // WiFi (needed for Matter)
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "📡 Starting WiFi...");
    wifi_sta_start();
    
    // Matter/HomeKit
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "📱 Initializing Matter...");
    
    if (aquarium_matter_init()) {
        ESP_LOGI(TAG, "✓ Matter ready");
    } else {
        ESP_LOGW(TAG, "⚠️ Matter failed");
    }
    
    // Start temperature monitoring
    aquarium_start();
    
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "🐠 READY!");
    ESP_LOGI(TAG, "   Temperature: every 5 seconds");
    ESP_LOGI(TAG, "   Range: 23°C - 28°C");
    ESP_LOGI(TAG, "   Pairing code: 20202021");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "");
    
    // LVGL main loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
    }
}
