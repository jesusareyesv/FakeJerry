#include "ble_hid.h"

#include <NimBLEDevice.h>
#include <NimBLEHIDDevice.h>

#include "config.h"

static const uint8_t REPORT_ID_KEYBOARD = 1;
static const uint8_t REPORT_ID_MOUSE = 2;

static const uint16_t APPEARANCE_KEYBOARD = 0x03C1;

static const uint8_t MODIFIER_LEFT_GUI = 0x08;
static const uint8_t KEYCODE_TAB = 0x2B;

static const uint8_t REPORT_MAP[] = {
    // Keyboard: modifiers, reserved byte, 6 key codes
    0x05, 0x01,                // Usage Page (Generic Desktop)
    0x09, 0x06,                // Usage (Keyboard)
    0xA1, 0x01,                // Collection (Application)
    0x85, REPORT_ID_KEYBOARD,  //   Report ID
    0x05, 0x07,                //   Usage Page (Keyboard)
    0x19, 0xE0,                //   Usage Minimum (Left Control)
    0x29, 0xE7,                //   Usage Maximum (Right GUI)
    0x15, 0x00,                //   Logical Minimum (0)
    0x25, 0x01,                //   Logical Maximum (1)
    0x75, 0x01,                //   Report Size (1)
    0x95, 0x08,                //   Report Count (8)
    0x81, 0x02,                //   Input (Data, Variable, Absolute)
    0x95, 0x01,                //   Report Count (1)
    0x75, 0x08,                //   Report Size (8)
    0x81, 0x01,                //   Input (Constant)
    0x95, 0x06,                //   Report Count (6)
    0x75, 0x08,                //   Report Size (8)
    0x15, 0x00,                //   Logical Minimum (0)
    0x25, 0x65,                //   Logical Maximum (101)
    0x05, 0x07,                //   Usage Page (Keyboard)
    0x19, 0x00,                //   Usage Minimum (0)
    0x29, 0x65,                //   Usage Maximum (101)
    0x81, 0x00,                //   Input (Data, Array)
    0xC0,                      // End Collection

    // Mouse: 3 buttons, relative X and Y
    0x05, 0x01,                // Usage Page (Generic Desktop)
    0x09, 0x02,                // Usage (Mouse)
    0xA1, 0x01,                // Collection (Application)
    0x85, REPORT_ID_MOUSE,     //   Report ID
    0x09, 0x01,                //   Usage (Pointer)
    0xA1, 0x00,                //   Collection (Physical)
    0x05, 0x09,                //     Usage Page (Buttons)
    0x19, 0x01,                //     Usage Minimum (1)
    0x29, 0x03,                //     Usage Maximum (3)
    0x15, 0x00,                //     Logical Minimum (0)
    0x25, 0x01,                //     Logical Maximum (1)
    0x95, 0x03,                //     Report Count (3)
    0x75, 0x01,                //     Report Size (1)
    0x81, 0x02,                //     Input (Data, Variable, Absolute)
    0x95, 0x01,                //     Report Count (1)
    0x75, 0x05,                //     Report Size (5)
    0x81, 0x01,                //     Input (Constant)
    0x05, 0x01,                //     Usage Page (Generic Desktop)
    0x09, 0x30,                //     Usage (X)
    0x09, 0x31,                //     Usage (Y)
    0x15, 0x81,                //     Logical Minimum (-127)
    0x25, 0x7F,                //     Logical Maximum (127)
    0x75, 0x08,                //     Report Size (8)
    0x95, 0x02,                //     Report Count (2)
    0x81, 0x06,                //     Input (Data, Variable, Relative)
    0xC0,                      //   End Collection
    0xC0,                      // End Collection
};

static NimBLEHIDDevice* hid;
static NimBLECharacteristic* keyboardInput;
static NimBLECharacteristic* mouseInput;
static volatile bool connected = false;

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* server, NimBLEConnInfo& connInfo) override {
    connected = true;
    Serial.println("BLE host connected");
  }

  void onDisconnect(NimBLEServer* server, NimBLEConnInfo& connInfo, int reason) override {
    connected = false;
    Serial.println("BLE host disconnected");
    NimBLEDevice::startAdvertising();
  }
};

static ServerCallbacks serverCallbacks;

void bleHidBegin() {
  NimBLEDevice::init(DEVICE_NAME);
  // Bonding so the host reconnects on its own; "just works" pairing, no PIN.
  NimBLEDevice::setSecurityAuth(true, false, true);

  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(&serverCallbacks, false);

  hid = new NimBLEHIDDevice(server);
  keyboardInput = hid->getInputReport(REPORT_ID_KEYBOARD);
  mouseInput = hid->getInputReport(REPORT_ID_MOUSE);

  hid->setManufacturer(DEVICE_MANUFACTURER);
  hid->setPnp(0x02, 0x303A, 0x0001, 0x0100);  // USB vendor ID source, Espressif VID
  hid->setHidInfo(0x00, 0x01);
  hid->setReportMap((uint8_t*)REPORT_MAP, sizeof(REPORT_MAP));
  hid->setBatteryLevel(100);
  hid->startServices();

  NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();
  advertising->setAppearance(APPEARANCE_KEYBOARD);
  advertising->addServiceUUID(hid->getHidService()->getUUID());
  advertising->setName(DEVICE_NAME);
  advertising->enableScanResponse(true);
  advertising->start();
}

bool bleHidConnected() {
  return connected;
}

void bleMouseMove(int8_t dx, int8_t dy) {
  if (!connected) return;
  uint8_t report[3] = {0, (uint8_t)dx, (uint8_t)dy};
  mouseInput->setValue(report, sizeof(report));
  mouseInput->notify();
}

static void sendKeyboardReport(uint8_t modifiers, uint8_t keycode) {
  uint8_t report[8] = {modifiers, 0, keycode, 0, 0, 0, 0, 0};
  keyboardInput->setValue(report, sizeof(report));
  keyboardInput->notify();
}

void bleKeyboardCmdTab() {
  if (!connected) return;
  sendKeyboardReport(MODIFIER_LEFT_GUI, 0);
  delay(40);
  sendKeyboardReport(MODIFIER_LEFT_GUI, KEYCODE_TAB);
  delay(40);
  sendKeyboardReport(MODIFIER_LEFT_GUI, 0);
  delay(40);
  sendKeyboardReport(0, 0);
}
