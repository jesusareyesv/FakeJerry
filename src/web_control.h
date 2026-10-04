#pragma once

// Joins WiFi (or starts the fallback access point) and starts the HTTP server.
void webControlBegin();

// Call from loop() to serve pending HTTP requests.
void webControlTick();
