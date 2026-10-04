#pragma once

#include <Arduino.h>

// Name shown in the Bluetooth pairing list, the fallback WiFi access point
// and the mDNS hostname (http://fakejerry.local).
#define DEVICE_NAME "FakeJerry"
#define MDNS_HOSTNAME "fakejerry"

// ESP32 DevKit: BOOT button pulls GPIO0 low, onboard LED is on GPIO2.
constexpr uint8_t PIN_BUTTON = 0;
constexpr uint8_t PIN_LED = 2;

// Defaults used until something else is saved from the web page.
constexpr uint32_t DEFAULT_MOUSE_INTERVAL_SEC = 240;
constexpr uint32_t DEFAULT_CMD_TAB_INTERVAL_SEC = 600;  // 0 disables Cmd+Tab

constexpr uint32_t MIN_INTERVAL_SEC = 5;
constexpr uint32_t MAX_INTERVAL_SEC = 3600;

// Each interval is randomised by +/- this percentage.
constexpr uint8_t INTERVAL_JITTER_PERCENT = 20;

// How far the pointer moves (it moves back right after).
constexpr int8_t MOUSE_NUDGE_PIXELS = 4;

constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
