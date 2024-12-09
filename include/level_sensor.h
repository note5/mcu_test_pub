#ifndef LEVEL_SENSOR_H
#define LEVEL_SENSOR_H
#include <Arduino.h>
#include <Wire.h>

#define US42_ADDRESS 0x70 // I2C address for the GY-US42V2 ultrasonic sensor

class I2cLevelSensor {
private:
    TwoWire *wire; // Pointer to I2C interface
    const uint8_t address; // I2C address of the sensor
    
public:
// constructor that takes twowire object and I2C address
    I2cLevelSensor(TwoWire &w = Wire, uint8_t addr = US42_ADDRESS);
    //initilize i2c communication returns true if address is found
    bool begin();
    //get raw values from the sensor
    uint16_t readDistance();
    //returns distance in cm
    float readDistanceInCM();
    float readAverageDistance(uint8_t samples = 10, uint16_t delayTime = 100);
};

#endif