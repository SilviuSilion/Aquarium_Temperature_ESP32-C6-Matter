#include "ds18b20.h"
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rom/ets_sys.h"

static inline void ow_drive_low(gpio_num_t pin){
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);
}
static inline void ow_release(gpio_num_t pin){
    gpio_set_direction(pin, GPIO_MODE_INPUT);
}
static inline int ow_read(gpio_num_t pin){
    return gpio_get_level(pin);
}


static void ow_init(gpio_num_t pin)
{
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << pin,
        .mode = GPIO_MODE_INPUT_OUTPUT_OD,   // open-drain!
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io);
    gpio_set_level(pin, 1);
}

static int ow_reset(gpio_num_t pin)
{
    int presence;

    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 0);
    ets_delay_us(480);

    gpio_set_level(pin, 1);
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    ets_delay_us(70);

    presence = (gpio_get_level(pin) == 0);   // DS18 trage low dacă e prezent
    ets_delay_us(410);

    return presence;
}

static void ow_write_bit(gpio_num_t pin, int v){
    ow_drive_low(pin);
    if (v) { ets_delay_us(6);  ow_release(pin); ets_delay_us(64); }
    else   { ets_delay_us(60); ow_release(pin); ets_delay_us(10); }
}

static int ow_read_bit(gpio_num_t pin){
    ow_drive_low(pin);
    ets_delay_us(6);
    ow_release(pin);
    ets_delay_us(9);
    int r = ow_read(pin);
    ets_delay_us(55);
    return r;
}

static void ow_write_byte(gpio_num_t pin, uint8_t b){
    for (int i=0;i<8;i++) ow_write_bit(pin, (b>>i)&1);
}
static uint8_t ow_read_byte(gpio_num_t pin){
    uint8_t r=0;
    for (int i=0;i<8;i++) r |= (uint8_t)(ow_read_bit(pin)<<i);
    return r;
}

static uint8_t crc8(const uint8_t *data, int len){
    uint8_t crc=0;
    for (int i=0;i<len;i++){
        uint8_t inbyte=data[i];
        for (int j=0;j<8;j++){
            uint8_t mix=(crc^inbyte)&1;
            crc>>=1;
            if(mix) crc^=0x8C;
            inbyte>>=1;
        }
    }
    return crc;
}

float ds18b20_read_temperature(gpio_num_t pin)
{
    ow_init(pin);

    if(!ow_reset(pin)) return NAN;

    ow_write_byte(pin, 0xCC); // SKIP ROM
    ow_write_byte(pin, 0x44); // CONVERT T
    vTaskDelay(pdMS_TO_TICKS(750));

    if(!ow_reset(pin)) return NAN;

    ow_write_byte(pin, 0xCC);
    ow_write_byte(pin, 0xBE); // READ SCRATCHPAD

    uint8_t l = ow_read_byte(pin);
    uint8_t h = ow_read_byte(pin);

    int16_t raw = (int16_t)((h << 8) | l);
    return (float)raw / 16.0f;
}


