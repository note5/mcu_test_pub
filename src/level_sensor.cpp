#include "level_sensor.h"

I2cLevelSensor::I2cLevelSensor(TwoWire &w, uint8_t addr)
    : wire(&w), address(addr), sensorState(IDLE),
      measureStartTime(0), lastCycleTime(0), latestDistance(-1.0) {}

bool I2cLevelSensor::begin()
{
    wire->begin();
    wire->beginTransmission(address);
    return (wire->endTransmission() == 0);
}

void I2cLevelSensor::update()
{
    unsigned long now = millis();

    if (sensorState == IDLE)
    {
        if (now - lastCycleTime >= READ_INTERVAL)
        {
            // trigger a new measurement
            wire->beginTransmission(address);
            wire->write(0x51);
            wire->endTransmission();
            measureStartTime = now;
            sensorState = MEASURING;
        }
    }
    else if (sensorState == MEASURING)
    {
        if (now - measureStartTime >= MEASURE_WAIT)
        {
            // read result
            wire->requestFrom(address, (uint8_t)2);
            if (wire->available() >= 2)
            {
                uint16_t raw = wire->read() << 8;
                raw |= wire->read();
                if (raw != 0xFFFF)
                {
                    latestDistance = (float)raw;
                }
            }
            lastCycleTime = now;
            sensorState = IDLE;
        }
    }
}

float I2cLevelSensor::getDistance()
{
    return latestDistance;
}
