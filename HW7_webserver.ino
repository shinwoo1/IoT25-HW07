#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <math.h>

#define SCAN_TIME 1
#define RSSI_WINDOW_SIZE 5

const char* ssid = "Me";
const char* password = "12345678";

BLEScan* pBLEScan;
WiFiServer server(80);

int rssiBuffer[RSSI_WINDOW_SIZE];
int rssiIndex = 0;
bool bufferFilled = false;
float currentDistance = 0.0;
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
      currentDistance = calculateDistance(avgRSSI);
      Serial.printf("RSSI: %d → AVG: %.1f dBm → 거리: %.2f m\n",
                    rssi, avgRSSI, currentDistance);
    }
  }
};

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWi-Fi 연결 완료. IP:");
  Serial.println(WiFi.localIP());
  server.begin();

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
}

void loop() {
  pBLEScan->start(SCAN_TIME, false);
  pBLEScan->clearResults();
  delay(200);

  WiFiClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        client.readStringUntil('\r');
        client.read();

        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println("Connection: close");
        client.println();
        client.println("<!DOCTYPE html><html><head>");
        client.println("<meta charset='UTF-8'>");
        client.println("<meta http-equiv='refresh' content='1'>");
        client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
        client.println("<title>BLE 거리 측정</title></head><body>");
        client.println("<h2>ESP32 BLE 거리 측정</h2>");
        client.printf("<p>현재 거리: <strong>%.2f m</strong></p>", currentDistance);
        client.println("</body></html>");
        break;
      }
    }
    client.stop();
  }
}
