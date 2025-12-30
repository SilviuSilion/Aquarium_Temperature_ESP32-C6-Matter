#!/bin/bash

# Flash script for Aquarium Temperature Monitor
# ESP32-C6-LCD-1.47

echo "🐠 Aquarium Temperature Monitor - Flash Script"
echo "=============================================="
echo ""

# Check if ESP-IDF is set up
if [ -z "$IDF_PATH" ]; then
    echo "Setting up ESP-IDF environment..."
    source ~/esp-idf/export.sh
fi

# Find USB port
echo "Searching for ESP32-C6..."
PORT=$(ls /dev/cu.usbserial-* 2>/dev/null | head -n1)

if [ -z "$PORT" ]; then
    PORT=$(ls /dev/cu.usb* 2>/dev/null | head -n1)
fi

if [ -z "$PORT" ]; then
    echo "❌ No USB device found!"
    echo "Please connect ESP32-C6 and try again"
    exit 1
fi

echo "✓ Found device: $PORT"
echo ""

# Build if needed
if [ ! -f "build/ESP32-C6-LCD-1.47-Test.bin" ]; then
    echo "Building project..."
    idf.py build || exit 1
    echo ""
fi

# Flash
echo "Flashing firmware..."
idf.py -p $PORT flash

# Monitor
echo ""
echo "Starting monitor (Ctrl+] to exit)..."
echo "=============================================="
idf.py -p $PORT monitor
