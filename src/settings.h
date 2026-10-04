#pragma once

#include <Arduino.h>

struct Settings {
  bool enabled;
  uint32_t mouseIntervalSec;
  uint32_t cmdTabIntervalSec;  // 0 = Cmd+Tab disabled
};

extern Settings settings;

void settingsLoad();
void settingsSave();
