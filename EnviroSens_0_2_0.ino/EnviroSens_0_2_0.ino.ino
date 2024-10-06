#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define SERVICE_UUID        BLEUUID((uint16_t)0x181A) // Environmental Sensing service
#define CHAR_TEMP_UUID      BLEUUID((uint16_t)0x2A6E) // Temperature characteristic
#define CHAR_PRESSURE_UUID  BLEUUID((uint16_t)0x2A6D) // Pressure characteristic
#define CHAR_HUMIDITY_UUID  BLEUUID((uint16_t)0x2A6F) // Humidity characteristic
#define CHAR_GAS_UUID       BLEUUID((uint16_t)0x2A70) // VOC characteristic (using this for gas resistance)

BLEServer* pServer = NULL;
BLECharacteristic* pTempCharacteristic = NULL;
BLECharacteristic* pPressureCharacteristic = NULL;
BLECharacteristic* pHumidityCharacteristic = NULL;
BLECharacteristic* pGasCharacteristic = NULL;
bool deviceConnected = false;
Adafruit_BME680 bme;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("Device connected");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("Device disconnected");
      pServer->startAdvertising(); // Restart advertising
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting ESP32 BME680 BLE Server");
  
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  // Create the BLE Device
  BLEDevice::init("ESP32_BME680");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics
  pTempCharacteristic = pService->createCharacteristic(
                      CHAR_TEMP_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pPressureCharacteristic = pService->createCharacteristic(
                      CHAR_PRESSURE_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pHumidityCharacteristic = pService->createCharacteristic(
                      CHAR_HUMIDITY_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pGasCharacteristic = pService->createCharacteristic(
                      CHAR_GAS_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Create BLE Descriptors
  pTempCharacteristic->addDescriptor(new BLE2902());
  pPressureCharacteristic->addDescriptor(new BLE2902());
  pHumidityCharacteristic->addDescriptor(new BLE2902());
  pGasCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  
  Serial.println("Starting advertising...");
  BLEDevice::startAdvertising();
  Serial.println("Advertising started. Waiting for connections...");
}

void loop() {
  if (deviceConnected) {
    // Read BME680 sensor data
    if (bme.performReading()) {
      uint16_t temperature = bme.temperature * 100;  // Temperature in 0.01 degrees Celsius
      uint32_t pressure = bme.pressure;              // Pressure in Pa
      uint16_t humidity = bme.humidity * 100;        // Humidity in 0.01 %
      uint32_t gas = bme.gas_resistance;             // Gas resistance in Ohms

      pTempCharacteristic->setValue((uint8_t*)&temperature, 2);
      pTempCharacteristic->notify();
      pPressureCharacteristic->setValue((uint8_t*)&pressure, 4);
      pPressureCharacteristic->notify();
      pHumidityCharacteristic->setValue((uint8_t*)&humidity, 2);
      pHumidityCharacteristic->notify();
      pGasCharacteristic->setValue((uint8_t*)&gas, 4);
      pGasCharacteristic->notify();

      Serial.println("Notified new sensor data:");
      Serial.printf("Temperature: %.2f °C\n", bme.temperature);
      Serial.printf("Pressure: %.2f hPa\n", bme.pressure / 100.0);
      Serial.printf("Humidity: %.2f %%\n", bme.humidity);
      Serial.printf("Gas: %d ohms\n", bme.gas_resistance);
    } else {
      Serial.println("Failed to perform reading :(");
    }
  }
  delay(2000);
}