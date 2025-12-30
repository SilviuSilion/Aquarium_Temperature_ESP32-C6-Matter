#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include <string.h>
#include <stdlib.h>

static const char *TAG = "WIFI_STA";

static TimerHandle_t s_reconnect_timer;

static void reconnect_timer_cb(TimerHandle_t xTimer)
{
    (void)xTimer;
    esp_err_t err = esp_wifi_connect();
    if(err != ESP_OK) {
        ESP_LOGW(TAG, "esp_wifi_connect (reconnect) failed: %s", esp_err_to_name(err));
    }
}

#define WIFI_CHANNEL 7
static const uint8_t WIFI_BSSID[6] = { 0x10, 0x7C, 0x61, 0xE2, 0xC8, 0x80 };

#define WIFI_SSID "I\xE2\x80\x99m Virus"
#define WIFI_PASS "Monika2025@NeagraSarului"

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    (void)arg;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *d = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGW(TAG, "Disconnected (reason=%d). Retrying in 2s...", d ? d->reason : -1);

        // NU vTaskDelay aici. Folosim timer.
        if(s_reconnect_timer) xTimerStart(s_reconnect_timer, 0);

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Connecting to BSSID 10:7C:61:E2:C8:80 (SSID may contain unicode)");
    }
}

static void wifi_scan_print(void)
{
    ESP_LOGI(TAG, "Connecting to BSSID 10:7C:61:E2:C8:80 (SSID may contain unicode)");

    wifi_scan_config_t scan_cfg = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time.active.min = 80,
        .scan_time.active.max = 200,
    };

    esp_err_t err = esp_wifi_scan_start(&scan_cfg, true); // blocking
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "scan_start failed: %s", esp_err_to_name(err));
        return;
    }

    uint16_t ap_num = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_num));
    ESP_LOGI(TAG, "Scan done. APs found: %u", ap_num);

    if (ap_num == 0) return;

    wifi_ap_record_t *recs = (wifi_ap_record_t *)calloc(ap_num, sizeof(wifi_ap_record_t));
    if (!recs) return;

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, recs));

    uint16_t n = ap_num > 30 ? 30 : ap_num;
    for (uint16_t i = 0; i < n; i++) {
        ESP_LOGI(TAG, "[%u] SSID='%s' RSSI=%d CH=%d AUTH=%d BSSID=%02X:%02X:%02X:%02X:%02X:%02X",
                 i, (char *)recs[i].ssid, recs[i].rssi, recs[i].primary, recs[i].authmode,
                 recs[i].bssid[0],recs[i].bssid[1],recs[i].bssid[2],recs[i].bssid[3],recs[i].bssid[4],recs[i].bssid[5]);
    }

    free(recs);
}

void wifi_sta_start(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler, NULL, NULL));
    if(!s_reconnect_timer) {
    s_reconnect_timer = xTimerCreate("reconn", pdMS_TO_TICKS(2000),
                                     pdFALSE, NULL, reconnect_timer_cb);
}
   
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // 1) Scan FIRST (no connect yet)
    wifi_scan_print();

    // 2) Configure and connect ONCE
    
   
   
    wifi_config_t wifi_config = { 0 };

    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));

    wifi_config.sta.bssid_set = 1;
    memcpy(wifi_config.sta.bssid, WIFI_BSSID, 6);
    wifi_config.sta.channel = WIFI_CHANNEL;

    wifi_config.sta.scan_method = WIFI_FAST_SCAN;
    wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;    
   

    /* Dacă SSID-ul are caractere Unicode (I’m), folosește BSSID-ul din scan */
    wifi_config.sta.bssid_set = 1;
    uint8_t bssid[6] = { 0x10, 0x7C, 0x61, 0xE2, 0xC8, 0x80 };   // <-- al tău din log
    memcpy(wifi_config.sta.bssid, bssid, 6);

    ESP_LOGI(TAG, "Connecting by BSSID %02X:%02X:%02X:%02X:%02X:%02X (SSID='%s')",
              bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5], WIFI_SSID);

    /* SET CONFIG înainte de CONNECT */
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    /* Connect */
    esp_err_t err = esp_wifi_connect();
    if (err != ESP_OK) {
         ESP_LOGE(TAG, "esp_wifi_connect failed: %s", esp_err_to_name(err));
    }
          // More tolerant
    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    wifi_config.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;

        // Lock to a specific AP (BSSID) so SSID punctuation can’t break us
    wifi_config.sta.bssid_set = 1;
    wifi_config.sta.bssid[0]=0x10;
    wifi_config.sta.bssid[1]=0x7C;
    wifi_config.sta.bssid[2]=0x61;
    wifi_config.sta.bssid[3]=0xE2;
    wifi_config.sta.bssid[4]=0xC8;
    wifi_config.sta.bssid[5]=0x80;
    wifi_config.sta.channel = 7; // optional, but speeds up connect

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

    ESP_LOGI(TAG, "Connecting to BSSID 10:7C:61:E2:C8:80 (SSID may contain unicode)");
    esp_wifi_connect(); // DO NOT ESP_ERROR_CHECK here
}
