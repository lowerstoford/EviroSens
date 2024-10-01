// SensorManager.h
#pragma once

#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <BH1750.h>
#include <Adafruit_MPU6050.h>

class SensorManager {
public:
    void begin();
    String getSensorData();

private:
    Adafruit_BME680 bme;
    BH1750 lightMeter;
    Adafruit_MPU6050 mpu;
};