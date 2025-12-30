# 🐠 Aquarium Temperature Monitor - Matter/HomeKit

## Proiect ESP32-C6 cu integrare Apple HomeKit

**Autori:** Claude & Silviu  
**Data:** 25 Decembrie 2024  
**Status:** ✅ FUNCȚIONAL

---

## 📋 Specificații Hardware

| Component | Detalii |
|-----------|---------|
| MCU | ESP32-C6-LCD-1.47 (8MB flash) |
| Display | ST7789 172x320px |
| Senzor | DS18B20 (GPIO 3) |
| LED RGB | WS2812 (GPIO 8) |
| WiFi | 802.11 b/g/n |
| Bluetooth | BLE 5.0 (pentru Matter pairing) |

---

## 🏠 Matter/HomeKit Integration

### Device Info
- **Manufacturer:** Claude&Silviu
- **Model:** Senzor Apa
- **Serial:** AQ-TEMP-001

### Pairing
1. Deschide **Home** app pe iPhone
2. Tap **+** → **Add Accessory**
3. Caută **"MATTER-3840"** sau introdu manual:
4. **Pairing Code:** `20202021`

---

## 🌡️ Funcționalitate

### Range Temperatură
| Stare | Temperatură | LED |
|-------|-------------|-----|
| 🔴 Prea rece | < 23°C | Roșu |
| 🌊 Normal | 23°C - 28°C | Ocean Blue → Verde |
| 🔴 Prea cald | > 28°C | Roșu |

### Caracteristici
- ✅ Citire temperatură la fiecare ~2 secunde
- ✅ Update HomeKit în timp real
- ✅ LED RGB cu gradient ocean
- ✅ Display LVGL cu font mare 48px
- ✅ WiFi auto-connect

---

## 📱 Notificări și Istoric

### Notificări (în Home app)
1. Deschide **Home** → **Automations**
2. Creează automatizare:
   - **Trigger:** "Senzor Apa" temperatura < 23°C
   - **Action:** Send Notification

### Istoric Temperatură
- HomeKit nu salvează istoric nativ
- **Recomandare:** Instalează **Eve app** (gratis) pentru grafice

---

## 🔧 Comenzi Build

```bash
# Activare ESP-IDF
cd /Volumes/Extensie/projects/Temperatura_Acvariu
source /Volumes/Extensie/esp-idf/export.sh

# Build
idf.py build

# Flash
idf.py -p /dev/cu.usbmodem112301 flash

# Monitor serial
idf.py -p /dev/cu.usbmodem112301 monitor

# Erase (pentru re-pairing)
idf.py -p /dev/cu.usbmodem112301 erase-flash
```

---

## 📁 Structură Fișiere Importante

```
Temperatura_Acvariu/
├── main/
│   ├── main.c                 # Entry point
│   ├── aquarium_controller.c  # DS18B20 + LED logic
│   ├── aquarium_controller.h
│   ├── aquarium_ui.c          # LVGL interface
│   ├── Matter/
│   │   ├── aquarium_matter.cpp  # Matter integration
│   │   └── aquarium_matter.h
│   ├── RGB/
│   │   ├── RGB.c              # WS2812 LED driver
│   │   └── RGB.h
│   └── LCD_Driver/            # Display drivers
├── sdkconfig.defaults         # Build configuration
├── partitions.csv             # Flash partitions (8MB)
└── CMakeLists.txt
```

---

## 🐛 Probleme Rezolvate

### 1. LED culori inversate
- **Problemă:** LED arăta verde în loc de roșu
- **Cauză:** WS2812 avea ordinea GRB nu RGB
- **Fix:** Inversat R și G în `Set_RGB()`

### 2. Temperatura nu se actualiza în HomeKit
- **Problemă:** Rămânea la 25°C
- **Cauză:** `aquarium_start()` nu era apelat
- **Fix:** Adăugat apelul în `main.c`

### 3. Matter compilation errors (GCC 14.2)
- **Problemă:** `Nullable.h` operator== ambiguous
- **Fix:** Patch manual în managed_components

### 4. mbedTLS HKDF missing
- **Fix:** `CONFIG_MBEDTLS_HKDF_C=y` în sdkconfig.defaults

---

## 📊 Configurare Matter

```c
// sdkconfig.defaults
CONFIG_DEVICE_VENDOR_NAME="Claude&Silviu"
CONFIG_DEVICE_PRODUCT_NAME="Senzor Apa"
CONFIG_DEVICE_VENDOR_ID=0xFFF1
CONFIG_DEVICE_PRODUCT_ID=0x8000
```

---

## 🎄 Note Finale

Proiect realizat de Crăciun 2024! 🎅

Funcționează perfect cu:
- iPhone Home app
- Automatizări HomeKit
- Eve app pentru istoric

**Enjoy your smart aquarium! 🐠**
