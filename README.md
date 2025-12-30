# 🐠 Aquarium Temperature Monitor

ESP32-C6 based aquarium temperature monitor with **Matter/HomeKit** support and a beautiful **Neon Glow UI**.

![Waveshare ESP32-C6-LCD-1.47](https://www.waveshare.com/w/upload/thumb/5/5e/ESP32-C6-LCD-1.47-details-intro.jpg/450px-ESP32-C6-LCD-1.47-details-intro.jpg)

## ✨ Features

- 🌡️ **DS18B20** waterproof temperature sensor with CRC8 validation
- 🏠 **Matter/HomeKit** integration for smart home control
- 📱 Works with **Apple Home**, **Google Home**, **Alexa**
- 🖥️ **1.47" LCD** display (172x320) with ST7789 driver
- 🎨 **Neon Glow UI** with Nemo fish icon
- ✏️ **Montserrat Thin 72px** font for elegant temperature display
- 💡 RGB LED status indicator
- 📶 WiFi/BLE commissioning support

## 🎯 Temperature Status

| Range | Status | Color |
|-------|--------|-------|
| < 23°C | COLD! | 🔴 Red |
| 23-28°C | OPTIMAL | 🟢 Green |
| > 28°C | HOT! | 🔴 Red |

## 🔧 Hardware

- **Board:** [Waveshare ESP32-C6-LCD-1.47](https://www.waveshare.com/esp32-c6-lcd-1.47.htm)
- **Display:** 172x320 IPS LCD (ST7789)
- **Sensor:** DS18B20 waterproof temperature sensor
- **GPIO3:** OneWire data pin for DS18B20

## 🛠️ Build & Flash

```bash
# Set up ESP-IDF (v5.4+)
source /path/to/esp-idf/export.sh

# Build
idf.py build

# Flash
idf.py -p /dev/ttyUSB0 flash monitor
```

## 📁 Project Structure

```
├── main/
│   ├── aquarium_controller.c  # Temperature reading & Matter
│   ├── aquarium_ui.c          # LVGL Neon Glow UI
│   ├── fonts/                 # Montserrat Thin fonts
│   ├── LVGL_UI/              # Nemo image assets
│   ├── LCD_Driver/           # ST7789 display driver
│   ├── RGB/                  # WS2812 LED control
│   └── Matter/               # Matter/HomeKit integration
├── components/
│   ├── ds18b20/              # Temperature sensor driver
│   └── lvgl__lvgl/           # LVGL graphics library
└── lv_conf.h                 # LVGL configuration
```

## 🏠 Matter/HomeKit Setup

1. Power on the device
2. Open **Apple Home** app (or Google Home/Alexa)
3. Scan the QR code or enter the setup code
4. The aquarium temperature will appear as a sensor

## 📜 License

MIT License - feel free to use for your own aquarium projects!

## 🙏 Credits

- Nemo icon from Finding Nemo
- [LVGL](https://lvgl.io/) - Graphics library
- [ESP-IDF](https://github.com/espressif/esp-idf) - Development framework
- [ESP-Matter](https://github.com/espressif/esp-matter) - Matter SDK
