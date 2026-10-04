#pragma once

#include <Arduino.h>

// Composite BLE HID device: one keyboard and one mouse in a single report map.

void bleHidBegin();
bool bleHidConnected();

void bleMouseMove(int8_t dx, int8_t dy);

// Presses Left GUI (Cmd on macOS) + Tab, then releases both.
void bleKeyboardCmdTab();
