#include <Arduino.h>

#include "ble_hid.h"
#include "config.h"
#include "jiggler.h"
#include "settings.h"
#include "web_control.h"

static const uint32_t DEBOUNCE_MS = 40;
static const uint32_t BLINK_MS = 500;

static bool buttonStable = HIGH;
static bool buttonLastRead = HIGH;
static uint32_t buttonChangedAt = 0;

static void buttonTick() {
  bool reading = digitalRead(PIN_BUTTON);
  uint32_t now = millis();

  if (reading != buttonLastRead) {
    buttonLastRead = reading;
    buttonChangedAt = now;
  }

  if (reading != buttonStable && now - buttonChangedAt >= DEBOUNCE_MS) {
    buttonStable = reading;
    if (buttonStable == LOW) {
      settings.enabled = !settings.enabled;
      settingsSave();
      jigglerReset();
      Serial.println(settings.enabled ? "enabled (button)" : "paused (button)");
    }
  }
}

// Solid = active, slow blink = enabled but no host connected, off = paused.
static void ledTick() {
  if (!settings.enabled) {
    digitalWrite(PIN_LED, LOW);
  } else if (bleHidConnected()) {
    digitalWrite(PIN_LED, HIGH);
  } else {
    digitalWrite(PIN_LED, (millis() / BLINK_MS) % 2);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);

  settingsLoad();
  bleHidBegin();
  webControlBegin();
  jigglerReset();
}

void loop() {
  buttonTick();
  ledTick();
  jigglerTick();
  webControlTick();
  delay(5);
}
