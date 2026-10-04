#include "web_control.h"

#include <Arduino.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFi.h>

#include "ble_hid.h"
#include "config.h"
#include "jiggler.h"
#include "settings.h"

static WebServer server(80);

static const char PAGE[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>)HTML" DEVICE_NAME R"HTML(</title>
<style>
  body { font-family: -apple-system, system-ui, sans-serif; max-width: 22rem; margin: 2rem auto; padding: 0 1rem; }
  h1 { font-size: 1.4rem; }
  #state { font-weight: 600; }
  button { font-size: 1rem; padding: .7rem 1rem; width: 100%; margin: .5rem 0 1.5rem; }
  label { display: block; margin: .8rem 0 .2rem; }
  input { font-size: 1rem; padding: .4rem; width: 100%; box-sizing: border-box; }
  small { color: #777; }
</style>
</head>
<body>
<h1>)HTML" DEVICE_NAME R"HTML(</h1>
<p>Status: <span id="state">...</span><br><small id="ble"></small></p>
<button id="toggle">...</button>
<form id="config">
  <label for="mouse">Mouse nudge every (seconds)</label>
  <input id="mouse" type="number" min="5" max="3600" required>
  <label for="cmdtab">Cmd+Tab every (seconds, 0 = off)</label>
  <input id="cmdtab" type="number" min="0" max="3600" required>
  <button type="submit">Save</button>
</form>
<script>
const $ = id => document.getElementById(id);
function show(s, fillForm) {
  $('state').textContent = s.enabled ? 'active' : 'paused';
  $('ble').textContent = s.connected ? 'Bluetooth host connected' : 'No Bluetooth host connected';
  $('toggle').textContent = s.enabled ? 'Pause' : 'Resume';
  if (fillForm) { $('mouse').value = s.mouseIntervalSec; $('cmdtab').value = s.cmdTabIntervalSec; }
}
async function call(path, opts, fillForm) {
  const r = await fetch(path, opts);
  if (r.ok) show(await r.json(), fillForm);
}
$('toggle').onclick = () => call('/api/toggle', {method: 'POST'}, false);
$('config').onsubmit = e => {
  e.preventDefault();
  const body = new URLSearchParams({mouse: $('mouse').value, cmdtab: $('cmdtab').value});
  call('/api/config', {method: 'POST', body}, true);
};
call('/api/status', {}, true);
setInterval(() => call('/api/status', {}, false), 3000);
</script>
</body>
</html>
)HTML";

static void sendStatus() {
  String json = "{\"enabled\":";
  json += settings.enabled ? "true" : "false";
  json += ",\"connected\":";
  json += bleHidConnected() ? "true" : "false";
  json += ",\"mouseIntervalSec\":";
  json += settings.mouseIntervalSec;
  json += ",\"cmdTabIntervalSec\":";
  json += settings.cmdTabIntervalSec;
  json += "}";
  server.send(200, "application/json", json);
}

static void handleToggle() {
  settings.enabled = !settings.enabled;
  settingsSave();
  jigglerReset();
  sendStatus();
}

static void handleConfig() {
  if (!server.hasArg("mouse") || !server.hasArg("cmdtab")) {
    server.send(400, "text/plain", "mouse and cmdtab are required");
    return;
  }

  long mouse = server.arg("mouse").toInt();
  long cmdTab = server.arg("cmdtab").toInt();
  bool mouseOk = mouse >= (long)MIN_INTERVAL_SEC && mouse <= (long)MAX_INTERVAL_SEC;
  bool cmdTabOk = cmdTab == 0 || (cmdTab >= (long)MIN_INTERVAL_SEC && cmdTab <= (long)MAX_INTERVAL_SEC);
  if (!mouseOk || !cmdTabOk) {
    server.send(400, "text/plain", "interval out of range");
    return;
  }

  settings.mouseIntervalSec = mouse;
  settings.cmdTabIntervalSec = cmdTab;
  settingsSave();
  jigglerReset();
  sendStatus();
}

#ifdef WIFI_SSID
static volatile uint8_t lastDisconnectReason = 0;

static void logJoinFailure() {
  Serial.printf("WiFi join to \"%s\" failed, disconnect reason %u: ", WIFI_SSID, lastDisconnectReason);
  switch (lastDisconnectReason) {
    case WIFI_REASON_NO_AP_FOUND:
      Serial.println("network not found");
      break;
    case WIFI_REASON_AUTH_FAIL:
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
    case WIFI_REASON_HANDSHAKE_TIMEOUT:
      Serial.println("wrong password or unsupported security mode");
      break;
    default:
      Serial.println("see wifi_err_reason_t in esp_wifi_types.h");
  }

  // The ESP32 radio is 2.4 GHz only, so a 5 GHz-only network never shows up here.
  // A scan fails while the station is still retrying the join, so stop it first.
  WiFi.disconnect();
  delay(200);
  int found = WiFi.scanNetworks();
  if (found < 0) {
    Serial.println("WiFi scan failed");
    return;
  }
  Serial.printf("Networks visible on 2.4 GHz: %d\n", found);
  for (int i = 0; i < found; i++) {
    Serial.printf("  %-32s ch %2d  %d dBm%s\n", WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i),
                  WiFi.SSID(i) == WIFI_SSID ? "  <- configured" : "");
  }
  WiFi.scanDelete();
}
#endif

static bool joinWifi() {
#ifdef WIFI_SSID
  WiFi.onEvent(
      [](WiFiEvent_t event, WiFiEventInfo_t info) { lastDisconnectReason = info.wifi_sta_disconnected.reason; },
      ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(MDNS_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    delay(250);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected, http://");
    Serial.println(WiFi.localIP());
    return true;
  }
  logJoinFailure();
  Serial.println("Falling back to access point");
  WiFi.disconnect(true);
#endif
  return false;
}

void webControlBegin() {
  if (!joinWifi()) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(DEVICE_NAME);
    Serial.print("Access point \"" DEVICE_NAME "\" up, http://");
    Serial.println(WiFi.softAPIP());
  }

  if (MDNS.begin(MDNS_HOSTNAME)) {
    MDNS.addService("http", "tcp", 80);
  }

  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", PAGE); });
  server.on("/api/status", HTTP_GET, sendStatus);
  server.on("/api/toggle", HTTP_POST, handleToggle);
  server.on("/api/config", HTTP_POST, handleConfig);
  server.begin();
}

void webControlTick() {
  server.handleClient();
}
