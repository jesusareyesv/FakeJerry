#include "settings.h"

#include <Preferences.h>

#include "config.h"

Settings settings;

static Preferences prefs;
static const char* NAMESPACE = "fakejerry";

void settingsLoad() {
  prefs.begin(NAMESPACE, true);
  settings.enabled = prefs.getBool("enabled", true);
  settings.mouseIntervalSec = prefs.getUInt("mouseSec", DEFAULT_MOUSE_INTERVAL_SEC);
  settings.cmdTabIntervalSec = prefs.getUInt("cmdTabSec", DEFAULT_CMD_TAB_INTERVAL_SEC);
  prefs.end();
}

void settingsSave() {
  prefs.begin(NAMESPACE, false);
  prefs.putBool("enabled", settings.enabled);
  prefs.putUInt("mouseSec", settings.mouseIntervalSec);
  prefs.putUInt("cmdTabSec", settings.cmdTabIntervalSec);
  prefs.end();
}
