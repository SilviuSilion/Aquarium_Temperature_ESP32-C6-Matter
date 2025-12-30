# 🐠 Aquarium Temperature Monitor

**ESP32-C6-LCD-1.47** - Professional aquarium temperature monitoring with HomeKit integration

## ✨ Features

### Hardware
- **ESP32-C6** - Wi-Fi 6 + BLE 5 + Matter support
- **1.47" LCD Display** (172x320) - ST7789 driver
- **DS18B20** Temperature Sensor (±0.5°C accuracy)
- **RGB LED** - WS2812 intelligent lighting
- **SD Card** slot for data logging (future)

### Software
- ✅ **Real-time temperature monitoring** (updates every 2 seconds)
- ✅ **Intelligent RGB LED control:**
  - 🔴 RED: Temperature <23°C or >28°C (danger!)
  - 🌊 OCEAN BLUE GRADIENT: 23-28°C (optimal range)
  - 💜 PURPLE PULSE: Sensor error
- ✅ **Beautiful LVGL UI:**
  - Ocean gradient background
  - Large, clear temperature display
  - Status indicators
  - Water wave icon 🌊
- ⏳ **Matter/HomeKit** integration (coming soon)
- ⏳ **Data logging** to SD card (coming soon)
- ⏳ **Temperature alerts** via notifications (coming soon)

## 📌 Hardware Connections

| Component | GPIO | Notes |
|-----------|------|-------|
| DS18B20 Data | GPIO3 | **Requires 4.7kΩ pull-up to 3.3V** |
| RGB LED | GPIO8 | WS2812/SK6812 compatible |
| LCD SCL | GPIO7 | SPI Clock |
| LCD SDA | GPIO6 | SPI MOSI |
| LCD RST | GPIO21 | Reset |
| LCD DC | GPIO15 | Data/Command |
| LCD CS | GPIO14 | Chip Select |
| LCD BL | GPIO22 | Backlight |

### ⚠️ CRITICAL: DS18B20 Wiring

```
DS18B20 Pin Layout (TO-92):
     ___
   /     \
  |  DS  |
  |18B20 |
   \_____/
    | | |
    1 2 3

Pin 1: GND    → ESP32 GND
Pin 2: DATA   → ESP32 GPIO3 + 4.7kΩ resistor to 3.3V
Pin 3: VDD    → ESP32 3.3V
```

**IMPORTANT:** The 4.7kΩ pull-up resistor is MANDATORY!

## 🚀 Quick Start

### 1. Clone & Navigate
```bash
cd /Volumes/Extensie/projects/Temperatura_Acvariu
```

### 2. Configure Wi-Fi
Edit `main/Wireless/wifi_sta.c`:
```c
#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
```

### 3. Build
```bash
source ~/esp-idf/export.sh
idf.py build
```

### 4. Flash
```bash
idf.py -p /dev/cu.usbserial-* flash monitor
```

Or use the provided flash script:
```bash
./flash.sh
```

## 🎨 LED Color Guide

| Temperature | LED Color | Meaning |
|-------------|-----------|---------|
| < 23.0°C | 🔴 Solid Red | TOO COLD - Check heater! |
| 23.0 - 28.0°C | 🌊 Ocean Blue Gradient | ✅ OPTIMAL RANGE |
| > 28.0°C | 🔴 Solid Red | TOO HOT - Check cooling! |
| Sensor Error | 💜 Pulsing Purple | Check sensor connection |

## 📱 HomeKit Integration (Coming Soon)

### Prerequisites
1. ESP-Matter SDK installed
2. Apple Home app on iPhone/iPad
3. HomeKit controller (HomePod, Apple TV, or iPad)

### Setup Steps
1. Build with Matter support:
```bash
idf.py menuconfig
# → Component config → ESP Matter → Enable
idf.py build flash
```

2. Open Apple Home app
3. Tap "+" → "Add Accessory"
4. Scan QR code shown on LCD display
5. Follow on-screen instructions

### HomeKit Features
- 🌡️ Temperature sensor (read-only)
- 🚨 Temperature alerts
- 📊 Historical data
- 🔔 Notifications when out of range
- 🏠 Automation support

## 🛠️ Troubleshooting

### Sensor reads NAN or --°C
- ✅ Check 4.7kΩ pull-up resistor (3.3V to DATA)
- ✅ Verify DS18B20 wiring (especially GND!)
- ✅ Try shorter cables (<3 meters recommended)
- ✅ Check GPIO3 configuration in code

### RGB LED doesn't light up
- ✅ Verify GPIO8 connection
- ✅ Check power supply (needs 5V for brightness)
- ✅ Test with RGB_Example() in main.c

### Display shows garbage
- ✅ Check SPI connections (GPIO6, 7, 14, 15, 21, 22)
- ✅ Verify LCD power (3.3V)
- ✅ Try adjusting backlight: `BK_Light(80)`

### Can't flash firmware
- ✅ Press and hold BOOT button while connecting USB
- ✅ Check USB cable (must support data, not just charging)
- ✅ Try different USB port
- ✅ Use: `idf.py -p /dev/cu.usbserial-* flash`

## 📂 Project Structure

```
Temperatura_Acvariu/
├── main/
│   ├── main.c                    # Main application
│   ├── aquarium_controller.c     # Temperature monitoring & LED control
│   ├── aquarium_controller.h
│   ├── aquarium_ui.c             # LVGL UI
│   ├── aquarium_ui.h
│   ├── LCD_Driver/               # ST7789 display driver
│   ├── LVGL_Driver/              # LVGL integration
│   ├── RGB/                      # WS2812 LED driver
│   ├── SD_Card/                  # SD card support
│   └── Wireless/                 # Wi-Fi connectivity
├── CMakeLists.txt
├── sdkconfig
└── README.md
```

## 🔧 Configuration

### Temperature Thresholds
Edit `main/aquarium_controller.h`:
```c
#define TEMP_MIN_NORMAL 23.0f  // Lower limit
#define TEMP_MAX_NORMAL 28.0f  // Upper limit
```

### Update Interval
Edit `main/aquarium_controller.c`:
```c
vTaskDelay(pdMS_TO_TICKS(2000));  // 2 seconds
```

### Display Brightness
Edit `main/main.c`:
```c
BK_Light(80);  // 0-100%
```

## 📊 Future Enhancements

- [ ] Matter/HomeKit full integration
- [ ] Temperature logging to SD card
- [ ] CSV export for analysis
- [ ] Email/push notifications
- [ ] Web dashboard
- [ ] Multiple sensor support
- [ ] Relay control for heater/cooler
- [ ] pH sensor integration
- [ ] Feeding schedule reminders

## 🤝 Contributing

Contributions welcome! Please open an issue or PR.

## 📝 License

MIT License - Free to use and modify

## 👨‍💻 Author

**Silviu Monica**
- Project: Temperatura Acvariu
- Hardware: ESP32-C6-LCD-1.47 (Waveshare)
- Created: December 2024

---

**Built with ❤️ for aquarium enthusiasts 🐠**
