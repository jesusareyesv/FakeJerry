#include "jiggler.h"

#include <Arduino.h>

#include "ble_hid.h"
#include "config.h"
#include "settings.h"

static uint32_t mouseTimerStart;
static uint32_t mouseWaitMs;
static uint32_t cmdTabTimerStart;
static uint32_t cmdTabWaitMs;

static uint32_t jitteredMs(uint32_t seconds) {
  uint32_t baseMs = seconds * 1000UL;
  uint32_t spread = baseMs / 100 * INTERVAL_JITTER_PERCENT;
  return baseMs - spread + random(2 * spread + 1);
}

void jigglerReset() {
  uint32_t now = millis();
  mouseTimerStart = now;
  mouseWaitMs = jitteredMs(settings.mouseIntervalSec);
  cmdTabTimerStart = now;
  cmdTabWaitMs = jitteredMs(settings.cmdTabIntervalSec);
}

void jigglerTick() {
  if (!settings.enabled || !bleHidConnected()) return;

  uint32_t now = millis();

  if (now - mouseTimerStart >= mouseWaitMs) {
    bleMouseMove(MOUSE_NUDGE_PIXELS, 0);
    delay(30);
    bleMouseMove(-MOUSE_NUDGE_PIXELS, 0);
    mouseTimerStart = now;
    mouseWaitMs = jitteredMs(settings.mouseIntervalSec);
    Serial.println("mouse nudge");
  }

  if (settings.cmdTabIntervalSec > 0 && now - cmdTabTimerStart >= cmdTabWaitMs) {
    bleKeyboardCmdTab();
    cmdTabTimerStart = now;
    cmdTabWaitMs = jitteredMs(settings.cmdTabIntervalSec);
    Serial.println("cmd+tab");
  }
}
