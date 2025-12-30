idf_component_register(
                         SRCS "main.c"
                              "temp_ui.c"
                              "LCD_Driver/Vernon_ST7789T/Vernon_ST7789T.c"
                              "LCD_Driver/ST7789.c"
                              "LVGL_Driver/LVGL_Driver.c"
                              "SD_Card/SD_SPI.c"
                              "RGB/RGB.c"
                              "Wireless/Wireless.c"

                              "Wireless/wifi_sta.c"

                         INCLUDE_DIRS
                              "./LCD_Driver/Vernon_ST7789T"
                              "./LCD_Driver"
                              "./LVGL_Driver"
                              "./LVGL_UI"
                              "./SD_Card"
                              "./RGB"
                              "./Wireless"
                              "."
                       )
