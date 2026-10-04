#pragma once

// Copy this file to include/secrets.h (it is gitignored) and fill it in.
// Without secrets.h, or if the network can't be joined, the board starts its
// own open access point named "FakeJerry" and serves the page at 192.168.4.1.

#define WIFI_SSID "your-network"
#define WIFI_PASSWORD "your-password"

// Optional: rename the device. Uncomment and edit.
// #define DEVICE_NAME "FakeJerry"          // Bluetooth name and fallback access point name
// #define DEVICE_MANUFACTURER "FakeJerry"  // Bluetooth manufacturer string, defaults to DEVICE_NAME
// #define MDNS_HOSTNAME "fakejerry"        // web page at http://<this>.local, no spaces
