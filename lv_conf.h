#pragma once

/* Ensure LVGL uses this config */
#define LV_CONF_INCLUDE_SIMPLE 1

/* --- Disable on-screen perf overlays --- */
#define LV_USE_PERF_MONITOR 0
#define LV_USE_MEM_MONITOR  0

/* Keep labels enabled (you need them) */
#define LV_USE_LABEL 1

/* ========================================
 * ENABLE LARGE FONTS FOR TEMPERATURE DISPLAY
 * ======================================== */

/* Enable built-in fonts */
#define LV_FONT_MONTSERRAT_8   0
#define LV_FONT_MONTSERRAT_10  0
#define LV_FONT_MONTSERRAT_12  0
#define LV_FONT_MONTSERRAT_14  1
#define LV_FONT_MONTSERRAT_16  1
#define LV_FONT_MONTSERRAT_18  0
#define LV_FONT_MONTSERRAT_20  1  // Medium size
#define LV_FONT_MONTSERRAT_22  0
#define LV_FONT_MONTSERRAT_24  1  // Large size
#define LV_FONT_MONTSERRAT_26  0
#define LV_FONT_MONTSERRAT_28  1  // Very large
#define LV_FONT_MONTSERRAT_30  0
#define LV_FONT_MONTSERRAT_32  1  // Extra large
#define LV_FONT_MONTSERRAT_34  0
#define LV_FONT_MONTSERRAT_36  1  // Huge!
#define LV_FONT_MONTSERRAT_38  0
#define LV_FONT_MONTSERRAT_40  0
#define LV_FONT_MONTSERRAT_42  0
#define LV_FONT_MONTSERRAT_44  0
#define LV_FONT_MONTSERRAT_46  0
#define LV_FONT_MONTSERRAT_48  1  // Maximum size for temperature!

/* Set default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_14
