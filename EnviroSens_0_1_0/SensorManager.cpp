// SensorManager.cpp
#include "SensorManager.h"

void SensorManager::begin() {
    Wire.begin();
    if (!bme.begin()) {
        Serial.println("Could not find BME680 sensor!");
    }
    lightMeter.begin();
    if (!mpu.begin()) {
        Serial.println("Could not find MPU6050 sensor!");
    }
}

String SensorManager::getSensorData() {
    String data = "";
    
    if (bme.performReading()) {
        data += "Temperature: " + String(bme.temperature) + " °C\n";
        data += "Pressure: " + String(bme.pressure / 100.0) + " hPa\n";
        data += "Humidity: " + String(bme.humidity) + " %\n";
        data += "Gas: " + String(bme.gas_resistance / 1000.0) + " KOhms\n";
    }

    float lux = lightMeter.readLightLevel();
    data += "Light: " + String(lux) + " lx\n";

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    data += "Accel: X:" + String(a.acceleration.x) + " Y:" + String(a.acceleration.y) + " Z:" + String(a.acceleration.z) + " m/s^2\n";
    data += "Gyro: X:" + String(g.gyro.x) + " Y:" + String(g.gyro.y) + " Z:" + String(g.gyro.z) + " rad/s\n";

    return data;
}