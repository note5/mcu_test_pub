#ifndef LEVEL_SENSOR_H
#define LEVEL_SENSOR_H
#include <Arduino.h>
#include <Wire.h>

#define US42_ADDRESS 0x70 // I2C address for the GY-US42V2 ultrasonic sensor

class I2cLevelSensor {
private:
    TwoWire *wire;
    const uint8_t address;

    enum State { IDLE, MEASURING };
    State sensorState;
    unsigned long measureStartTime;
    unsigned long lastCycleTime;
    float latestDistance;

    static const unsigned long MEASURE_WAIT = 70;
    static const unsigned long READ_INTERVAL = 2000;

public:
    I2cLevelSensor(TwoWire &w = Wire, uint8_t addr = US42_ADDRESS);
    bool begin();
    // call every loop iteration - non-blocking
    void update();
    // get the latest reading
    float getDistance();
};

#endif