#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

void setup() {
  Serial.begin(115200);
  BLEDevice::init("HW7_SERVER");

  BLEServer* pServer = BLEDevice::createServer();
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();

  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE 광고 중: HW7_SERVER");
}

void loop() {
  delay(2000);
}
