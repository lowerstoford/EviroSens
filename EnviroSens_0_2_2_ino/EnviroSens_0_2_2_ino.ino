#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <BH1750.h>
#include <MPU6050.h>

// I2C addresses
#define BME680_ADDR 0x76

#define BH1750_ADDR 0x23
#define MPU6050_ADDR 0x68
#define DISPLAY_NODE_ADDR 0x55  // Address for the Display Node

#define I2C_SDA 21
#define I2C_SCL 22

// Create sensor objects
Adafruit_BME680 bme;
BH1750 lightMeter;
MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Initialize BME680
  if (!bme.begin(BME680_ADDR)) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Initialize BH1750
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, BH1750_ADDR)) {
    Serial.println("Could not find a valid BH1750 sensor, check wiring!");
    while (1);
  }

  // Initialize MPU6050
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    while (1);
  }

  Serial.println("All sensors initialized successfully!");
}

void loop() {
  // Read sensor data
  bme.performReading();
  float lux = lightMeter.readLightLevel();
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Print sensor data to Serial (for debugging)
  Serial.println("Sensor Readings:");
  Serial.printf("Temperature: %.2f°C\n", bme.temperature);
  Serial.printf("Humidity: %.2f%%\n", bme.humidity);
  Serial.printf("Pressure: %.2f hPa\n", bme.pressure / 100.0);
  Serial.printf("Gas: %.2f KOhms\n", bme.gas_resistance / 1000.0);
  Serial.printf("Light: %.2f lux\n", lux);
  Serial.printf("Accel: X=%d, Y=%d, Z=%d\n", ax, ay, az);
  Serial.printf("Gyro: X=%d, Y=%d, Z=%d\n", gx, gy, gz);

  // Send data to Display Node
  Wire.beginTransmission(DISPLAY_NODE_ADDR);
  Wire.write((uint8_t*)&bme.temperature, 4);
  Wire.write((uint8_t*)&bme.humidity, 4);
  Wire.write((uint8_t*)&bme.pressure, 4);
  Wire.write((uint8_t*)&bme.gas_resistance, 4);
  Wire.write((uint8_t*)&lux, 4);
  Wire.write((uint8_t*)&ax, 2);
  Wire.write((uint8_t*)&ay, 2);
  Wire.write((uint8_t*)&az, 2);
  Wire.write((uint8_t*)&gx, 2);
  Wire.write((uint8_t*)&gy, 2);
  Wire.write((uint8_t*)&gz, 2);
  Wire.endTransmission();

  delay(5000);  // Wait for 5 seconds before next reading
}