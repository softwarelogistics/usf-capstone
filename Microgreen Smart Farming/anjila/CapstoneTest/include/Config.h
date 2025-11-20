#pragma once

// ---------- Firmware IDs ----------
#define HARDWARE_VERSION "1.0.0"
#define FIRMWARE_VERSION "0.1.0"
#define FIRMWARE_SKU     "MGS-Sensor"

// ---------- Board/Sensor config ----------
#define TEMP_SNSR_BOARD_V3

// MH-Z19B on UART2
static constexpr int RXD2 = 16;  // MH-Z19 TX -> ESP32 RX2
static constexpr int TXD2 = 17;  // MH-Z19 RX -> ESP32 TX2

// DS18B20 OneWire
static constexpr int ONE_WIRE_BUS = 4;

// DHT22
static constexpr int DHT22_PIN = 26;

// Sampling 
static constexpr unsigned long SAMPLE_PERIOD_MS = 5000;


