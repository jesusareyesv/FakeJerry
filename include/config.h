#pragma once

#include <Arduino.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif

// Each of these can be overridden from secrets.h.

// Name shown in the Bluetooth pairing list and used for the fallback WiFi
// access point.
#ifndef DEVICE_NAME
#define DEVICE_NAME "FakeJerry"
#endif

// Manufacturer string reported over Bluetooth.
#ifndef DEVICE_MANUFACTURER
#define DEVICE_MANUFACTURER DEVICE_NAME
#endif

// The web page is served at http://<MDNS_HOSTNAME>.local
#ifndef MDNS_HOSTNAME
#define MDNS_HOSTNAME "fakejerry"
#endif

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
