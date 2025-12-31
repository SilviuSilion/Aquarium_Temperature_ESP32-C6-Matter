/**
 * @file aquarium_ui.c
 * @brief Neon Glow UI - Nemo Edition
 * Display: 172x320, Design from neon_nemo_2.html
 */

#include "aquarium_ui.h"
#include "aquarium_controller.h"
#include "lvgl.h"
#include "LVGL_UI/nemo_img.h"
#include "esp_log.h"
#include <stdio.h>
#include <math.h>

static const char *TAG = "UI";

// UI elements
static lv_obj_t *nemo_img_obj = NULL;
static lv_obj_t *temp_int_label = NULL;
static lv_obj_t *temp_dec_label = NULL;
static lv_obj_t *celsius_label = NULL;
static lv_obj_t *status_dot = NULL;
static lv_obj_t *status_text = NULL;

// Colors
#define COLOR_BG          lv_color_hex(0x0A0A0A)
#define COLOR_CYAN        lv_color_hex(0x00D4FF)  // Same for temp and decimal
#define COLOR_GREEN       lv_color_hex(0x22C55E)
#define COLOR_RED         lv_color_hex(0xEF4444)
#define COLOR_GRAY        lv_color_hex(0x555555)  // CELSIUS darker like mockup

// Fonts - Custom thin fonts for temperature
LV_FONT_DECLARE(font_temp_72);  // Montserrat Thin 72px
LV_FONT_DECLARE(font_temp_36);  // Montserrat Thin 36px  
LV_FONT_DECLARE(lv_font_montserrat_14);


// Pulse animation for status dot
static void pulse_anim_cb(void *var, int32_t v) {
    lv_obj_set_style_opa((lv_obj_t*)var, v, 0);
}

static void ui_update_timer_cb(lv_timer_t *timer) {
    (void)timer;
    
    float temp = aquarium_get_last_temperature();
    bool valid = aquarium_is_temperature_valid();
    
    if (!valid || isnan(temp)) {
        lv_label_set_text(temp_int_label, "--");
        lv_label_set_text(temp_dec_label, ".-°");
        lv_obj_set_style_text_color(temp_int_label, COLOR_GRAY, 0);
        lv_obj_set_style_text_color(temp_dec_label, COLOR_GRAY, 0);
        lv_label_set_text(status_text, "SENSOR?");
        lv_obj_set_style_text_color(status_text, COLOR_RED, 0);
        lv_obj_set_style_bg_color(status_dot, COLOR_RED, 0);
    } else {
        int temp_int = (int)temp;
        int temp_frac = (int)((temp - temp_int) * 10);
        if (temp_frac < 0) temp_frac = -temp_frac;
        
        char buf_int[8], buf_dec[16];
        snprintf(buf_int, sizeof(buf_int), "%d", temp_int);
        snprintf(buf_dec, sizeof(buf_dec), ".%d°", temp_frac);
        
        lv_label_set_text(temp_int_label, buf_int);
        lv_label_set_text(temp_dec_label, buf_dec);
        
        // Re-align decimal after integer
        lv_obj_align_to(temp_dec_label, temp_int_label, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, -12);
        
        if (temp < TEMP_MIN_NORMAL) {
            lv_obj_set_style_text_color(temp_int_label, COLOR_RED, 0);
            lv_obj_set_style_text_color(temp_dec_label, COLOR_RED, 0);
            lv_label_set_text(status_text, "COLD!");
            lv_obj_set_style_text_color(status_text, COLOR_RED, 0);
            lv_obj_set_style_bg_color(status_dot, COLOR_RED, 0);
        } else if (temp > TEMP_MAX_NORMAL) {
            lv_obj_set_style_text_color(temp_int_label, COLOR_RED, 0);
            lv_obj_set_style_text_color(temp_dec_label, COLOR_RED, 0);
            lv_label_set_text(status_text, "HOT!");
            lv_obj_set_style_text_color(status_text, COLOR_RED, 0);
            lv_obj_set_style_bg_color(status_dot, COLOR_RED, 0);
        } else {
            lv_obj_set_style_text_color(temp_int_label, COLOR_CYAN, 0);
            lv_obj_set_style_text_color(temp_dec_label, COLOR_CYAN, 0);
            lv_label_set_text(status_text, "OPTIMAL");
            lv_obj_set_style_text_color(status_text, COLOR_GREEN, 0);
            lv_obj_set_style_bg_color(status_dot, COLOR_GREEN, 0);
        }
    }
}


void aquarium_ui_init(void) {
    ESP_LOGI(TAG, "Creating Neon Glow UI - Nemo Edition...");
    
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, COLOR_BG, 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    
    // ========== NEMO IMAGE (bigger, lower) ==========
    nemo_img_obj = lv_img_create(scr);
    lv_img_set_src(nemo_img_obj, &nemo_img);
    lv_obj_align(nemo_img_obj, LV_ALIGN_TOP_MID, 0, 25);
    
    // ========== TEMPERATURE INTEGER (BIG - 72px thin) ==========
    temp_int_label = lv_label_create(scr);
    lv_label_set_text(temp_int_label, "--");
    lv_obj_set_style_text_font(temp_int_label, &font_temp_72, 0);
    lv_obj_set_style_text_color(temp_int_label, COLOR_CYAN, 0);
    lv_obj_align(temp_int_label, LV_ALIGN_CENTER, -25, -10);
    
    // ========== TEMPERATURE DECIMAL (36px thin) ==========
    temp_dec_label = lv_label_create(scr);
    lv_label_set_text(temp_dec_label, ".-°");
    lv_obj_set_style_text_font(temp_dec_label, &font_temp_36, 0);
    lv_obj_set_style_text_color(temp_dec_label, COLOR_CYAN, 0);
    lv_obj_align_to(temp_dec_label, temp_int_label, LV_ALIGN_OUT_RIGHT_BOTTOM, 0, -12);

    
    // ========== CELSIUS label ==========
    celsius_label = lv_label_create(scr);
    lv_label_set_text(celsius_label, "CELSIUS");
    lv_obj_set_style_text_font(celsius_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(celsius_label, COLOR_GRAY, 0);
    lv_obj_set_style_text_letter_space(celsius_label, 3, 0);
    lv_obj_align(celsius_label, LV_ALIGN_CENTER, 0, 55);
    
    // ========== STATUS DOT (pulsing) ==========
    status_dot = lv_obj_create(scr);
    lv_obj_set_size(status_dot, 10, 10);
    lv_obj_set_style_radius(status_dot, 5, 0);
    lv_obj_set_style_bg_color(status_dot, COLOR_GREEN, 0);
    lv_obj_set_style_bg_opa(status_dot, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(status_dot, 0, 0);
    lv_obj_set_style_shadow_color(status_dot, COLOR_GREEN, 0);
    lv_obj_set_style_shadow_width(status_dot, 15, 0);
    lv_obj_set_style_shadow_spread(status_dot, 2, 0);
    lv_obj_align(status_dot, LV_ALIGN_BOTTOM_MID, -40, -35);
    
    // Pulse animation
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, status_dot);
    lv_anim_set_values(&a, 255, 128);
    lv_anim_set_time(&a, 1000);
    lv_anim_set_playback_time(&a, 1000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_exec_cb(&a, pulse_anim_cb);
    lv_anim_start(&a);
    
    // ========== STATUS TEXT ==========
    status_text = lv_label_create(scr);
    lv_label_set_text(status_text, "...");
    lv_obj_set_style_text_font(status_text, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(status_text, COLOR_GREEN, 0);
    lv_obj_set_style_text_letter_space(status_text, 2, 0);
    lv_obj_align_to(status_text, status_dot, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
    
    // Update timer (500ms - sufficient for 5s temp updates)
    lv_timer_create(ui_update_timer_cb, 500, NULL);
    
    ESP_LOGI(TAG, "Neon Glow UI ready");
}
