#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <MPU6050.h>
#include <BH1750.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define LED_BUILTIN 2
#define LED_RED 19
#define LED_GREEN 18
#define LED_BLUE 5
#define BUTTON_A 17
#define I2C_SDA 21
#define I2C_SCL 22

Adafruit_BME680 bme;
MPU6050 mpu;
BH1750 lightMeter;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
unsigned long lastTime = 0;
unsigned long timerDelay = 3000;

int16_t ax, ay, az;
int16_t gx, gy, gz;

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUTTON_A, INPUT_PULLUP);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  mpu.initialize();
  lightMeter.begin();

  BLEDevice::init("ESP32_Sensor_Pack");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(BLEUUID((uint16_t)0x181A));
  pCharacteristic = pService->createCharacteristic(
                      BLEUUID((uint16_t)0x2A6E),
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  pServer->getAdvertising()->start();
}

void loop() {
  if (deviceConnected) {
    if (millis() - lastTime > timerDelay) {
      lastTime = millis();
      String data = getSensorData();
      pCharacteristic->setValue(data.c_str());
      pCharacteristic->notify();
      Serial.println(data);
    }
  }
}

String getSensorData() {
  bme.performReading();
  float temp = bme.temperature;
  float hum = bme.humidity;
  float pres = bme.pressure / 100.0;
  float gas = bme.gas_resistance / 1000.0;
  float light = lightMeter.readLightLevel();
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  String data = "Temp: " + String(temp) + " C, Hum: " + String(hum) + " %, Pres: " + String(pres) + " hPa, Gas: " + String(gas) + " KOhms, Light: " + String(light) + " lx, Accel: " + String(ax) + ", " + String(ay) + ", " + String(az) + ", Gyro: " + String(gx) + ", " + String(gy) + ", " + String(gz);
  return data;
}
