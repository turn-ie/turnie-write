#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "WS_Flow.h"
#include <Preferences.h>

#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcd1234-ab12-cd34-ef56-123456789abc"

Preferences preferences;   // 永続化用

char Text[256];  // 受信テキスト格納
bool newTextReceived = false;  // 新しいテキスト受信フラグ

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        Serial.print("Received: ");
        Serial.println(rxValue);

        // Textにコピー
        rxValue.toCharArray(Text, sizeof(Text));

        // EEPROMに保存（Preferences）
        preferences.putString("text", rxValue);

        // フラッシュ要求
        newTextReceived = true;
      }
    }
};

void setup() {
  Serial.begin(115200);

  // Preferences初期化
  preferences.begin("matrix", false);

  // 前回保存した文字列を読み込み
  String savedText = preferences.getString("text", "turnie");
  savedText.toCharArray(Text, sizeof(Text));

  // BLE初期化
  BLEDevice::init("ESP32S3-BLE");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_READ
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("BLE ready. Waiting for text...");

  // LEDマトリクス初期化
  Matrix_Init();
}


void loop() {
  if (newTextReceived) {
    flashMatrix();
    newTextReceived = false;
  }

  Text_Flow(Text);  // 常に最新のTextをスクロール表示
  delay(100);
}
