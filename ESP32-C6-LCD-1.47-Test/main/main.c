/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "ST7789.h"
#include "SD_SPI.h"
#include "RGB.h"
#include "Wireless.h"
#include "temp_ui.h"

void app_main(void)
{
    // pornește Wi-Fi STA (SSID/PASS în wifi_sta.c)
    wifi_sta_start();

    Flash_Searching();
    RGB_Init();
    RGB_Example();
    SD_Init();          // SD must be initialized behind the LCD
    LCD_Init();
    BK_Light(50);

    LVGL_Init();
    temp_ui_start();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));
        lv_timer_handler();
    }
}
