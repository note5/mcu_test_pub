#include "level_sensor.h"

I2cLevelSensor::I2cLevelSensor(TwoWire &w, uint8_t addr)
    : wire(&w), address(addr) {}

bool I2cLevelSensor::begin()
{
    wire->begin();
    wire->beginTransmission(address);
    return (wire->endTransmission() == 0);
}

uint16_t I2cLevelSensor::readDistance()
{
    wire->beginTransmission(address);
    wire->write(0x51); // Command 0x51: Start single measurement
    wire->endTransmission();

    delay(70); // Wait for measurement

    wire->requestFrom(address, (uint8_t)2);

    if (wire->available() >= 2)
    {
        uint16_t distance = wire->read() << 8; // High byte
        distance |= wire->read();              // Low byte
        return distance;
    }

    return 0xFFFF; // Return max value if read fails
}

float I2cLevelSensor::readDistanceInCM()
{
    uint16_t rawDistance = readDistance();
    if (rawDistance == 0xFFFF)
    {
        return -1.0; // Indicate error
    }
    // Return raw value (already in centimeters for this sensor)
    return (float)rawDistance;
}

float I2cLevelSensor::readAverageDistance(uint8_t samples, uint16_t delayTime)
{
    float sum = 0;
    uint8_t validSamples = 0;

    for (uint8_t i = 0; i < samples; i++)
    {
        float distance = readDistanceInCM();
        if (distance > 0)
        { // Only count valid readings
            sum += distance;
            validSamples++;
        }
        delay(delayTime);
    }
    // Check if we got any valid readings
    if (validSamples == 0)
    {
        return -1.0; // Indicate error
    }

    return sum / validSamples;
}
