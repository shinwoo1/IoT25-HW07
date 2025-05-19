#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <math.h>

#define SCAN_TIME 1
#define RSSI_WINDOW_SIZE 5
#define LED_PIN 26
#define DIST_THRESHOLD 1.5

BLEScan* pBLEScan;
int rssiBuffer[RSSI_WINDOW_SIZE];
int rssiIndex = 0;
bool bufferFilled = false;
bool deviceDetected = false;

float calculateDistance(float rssi, int txPower = -65, float n = 2.0) {
  return pow(10.0, (txPower - rssi) / (10.0 * n));
}

float getAveragedRSSI(int newRSSI) {
  rssiBuffer[rssiIndex] = newRSSI;
  rssiIndex = (rssiIndex + 1) % RSSI_WINDOW_SIZE;
  if (rssiIndex == 0) bufferFilled = true;

  int sum = 0;
  int count = bufferFilled ? RSSI_WINDOW_SIZE : rssiIndex;
  for (int i = 0; i < count; i++) sum += rssiBuffer[i];
  return sum / (float)count;
}

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == "HW7_SERVER") {
      if (!deviceDetected) {
        Serial.println("BLE 서버에 연결됨!");
        deviceDetected = true;
      }
      int rssi = advertisedDevice.getRSSI();
      float avgRSSI = getAveragedRSSI(rssi);
      float distance = calculateDistance(avgRSSI);
      Serial.printf("AVG RSSI: %.1f dBm → 추정 거리: %.2f m → ", avgRSSI, distance);

      if (distance < DIST_THRESHOLD) {
        Serial.println("→ 가까움: LED ON");
        digitalWrite(LED_PIN, HIGH);
      } else {
        Serial.println("→ 멂: LED OFF");
        digitalWrite(LED_PIN, LOW);
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
}

void loop() {
  pBLEScan->start(SCAN_TIME, false);
  pBLEScan->clearResults();
  delay(200);
}
