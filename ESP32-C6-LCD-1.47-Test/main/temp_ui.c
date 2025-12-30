#include <stdio.h>
#include <time.h>
#include <math.h>
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"

#include "esp_sntp.h"


#include "lvgl.h"
#include "ds18b20.h"
#ifndef DS18B20_PIN
#define DS18B20_PIN GPIO_NUM_3   // rămâne 3 (cum ai sudat)
#endif

static float   g_last_good_temp = NAN;
static int64_t g_last_good_us   = 0;

static const char *TAG = "TEMPUI";

static lv_obj_t *label_temp = NULL;
static lv_obj_t *label_date;
static lv_obj_t *label_time;
static lv_obj_t *label_temp;
static lv_obj_t *pill;

static float last_temp = NAN;

/* ---------- TIME (SNTP) ---------- */

static void time_sync_init(void)
{
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_init();

    ESP_LOGI(TAG, "SNTP init...");
}

static void time_wait_task(void *arg)
{
    (void)arg;
    time_sync_init();

    // Așteaptă până se setează anul > 2023
    for (int i = 0; i < 30; i++) {
        time_t now = time(NULL);
        struct tm t;
        localtime_r(&now, &t);
        if (t.tm_year >= (2023 - 1900)) {
            ESP_LOGI(TAG, "Time set OK");
            vTaskDelete(NULL);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_LOGW(TAG, "Time not set (still running without clock)");
    vTaskDelete(NULL);
}

/* ---------- DS18B20 task ---------- */

static void ds18_task(void *arg)
{
    (void)arg;
    while (1) {
        float t = ds18b20_read_temperature(DS18B20_PIN);

        if (!isnan(t)) {
            g_last_good_temp = t;
            g_last_good_us = esp_timer_get_time();
        } else {
            ESP_LOGW(TAG, "DS18B20: read failed (NAN) - keeping last value");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

/* ---------- UI ---------- */

static void ui_build(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    // Background gradient (ocean-like)
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x04101B), 0);
    lv_obj_set_style_bg_grad_color(scr, lv_color_hex(0x0A3B5E), 0);
    lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_VER, 0);

    // Header pill (sus) - un “badge” semitransparent
    pill = lv_obj_create(scr);
    lv_obj_set_size(pill, 160, 34);
    lv_obj_align(pill, LV_ALIGN_TOP_MID, 0, 6);
    lv_obj_set_style_radius(pill, 12, 0);
    lv_obj_set_style_bg_color(pill, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(pill, LV_OPA_40, 0);
    lv_obj_set_style_border_width(pill, 0, 0);
    lv_obj_clear_flag(pill, LV_OBJ_FLAG_SCROLLABLE);

    // Date (sus stânga în pill)
    label_date = lv_label_create(pill);
    lv_obj_set_style_text_color(label_date, lv_color_hex(0xBFEFFF), 0);
    lv_obj_set_style_text_font(label_date, &lv_font_montserrat_12, 0);
    if (label_temp) lv_label_set_text(label_temp, "--.--°C");
    


    lv_obj_align(label_date, LV_ALIGN_LEFT_MID, 8, -6);

    // Time (sub dată, tot în pill)
    label_time = lv_label_create(pill);
    lv_obj_set_style_text_color(label_time, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_12, 0);
    lv_label_set_text(label_time, "--:--");
    lv_obj_align(label_time, LV_ALIGN_LEFT_MID, 8, 8);

    // Temp mare (centru)
    label_temp = lv_label_create(scr);
    lv_obj_set_style_text_color(label_temp, lv_color_hex(0xE9FCFF), 0);
    lv_obj_set_style_text_font(label_temp, &lv_font_montserrat_16, 0);
    lv_label_set_text(label_temp, "----");
    lv_obj_align(label_temp, LV_ALIGN_CENTER, 0, 10);

    // Mărim vizual temperatura (fără font 42): scale pe obiect
    // (LVGL îl suportă, dacă nu, doar ignoră)
    lv_obj_set_style_transform_zoom(label_temp, 512, 0); // ~2.0x (256=1.0x)
}

static void ui_update(void)
{
    // Date/time
    time_t now = time(NULL);
    struct tm t;
    localtime_r(&now, &t);

    char db[16];
    char tb[8];

    if (t.tm_year < (2023 - 1900)) {
        snprintf(db, sizeof(db), "--.--.----");
        snprintf(tb, sizeof(tb), "--:--");
    } else {
        strftime(db, sizeof(db), "%d.%m.%Y", &t);
        strftime(tb, sizeof(tb), "%H:%M", &t);
    }

    lv_label_set_text(label_date, db);
    lv_label_set_text(label_time, tb);

    // Temp (nu trecem pe ---- la un eșec scurt)
    int64_t age_us = (g_last_good_us == 0) ? INT64_MAX : (esp_timer_get_time() - g_last_good_us);

    char sb[24];
    if (!isnan(g_last_good_temp) && age_us < 10LL * 1000000LL) {
        snprintf(sb, sizeof(sb), "%.1f°C", g_last_good_temp);
    } else {
        snprintf(sb, sizeof(sb), "----");
    }
    lv_label_set_text(label_temp, sb);
}

static void ui_timer_cb(lv_timer_t *t)
{
    (void)t;

    float temp_c = ds18b20_read_temperature(DS18B20_PIN);

    if(!isnan(temp_c)) {
        last_temp = temp_c;
        if(label_temp) lv_label_set_text_fmt(label_temp, "%.2f°C", temp_c);
    } else {
        ESP_LOGW("TEMPUI", "DS18B20: read failed (NAN) - keeping last value");
    }
}

void temp_ui_start(void)
{
    ui_build();
    ui_update();
    label_temp = lv_label_create(lv_scr_act());
    lv_label_set_text(label_temp, "--.--°C");
    lv_timer_create(ui_timer_cb, 2000, NULL);
    // Task DS + Task NTP
    xTaskCreate(ds18_task, "ds18", 4096, NULL, 5, NULL);
    xTaskCreate(time_wait_task, "ntp", 4096, NULL, 4, NULL);

    lv_timer_create(ui_timer_cb, 500, NULL);
}
