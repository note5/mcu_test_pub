#include "hc_sr05.h"

HcSr05::HcSr05(uint8_t trigPin, const uint8_t *echoes, uint8_t count)
    : triggerPin(trigPin), sensorCount(min(count, MAX_SENSORS)),
      currentSensor(0), lastReadTime(0)
{
    for (uint8_t i = 0; i < sensorCount; i++)
    {
        echoPins[i] = echoes[i];
        distances[i] = -1.0;
    }
}

void HcSr05::begin()
{
    pinMode(triggerPin, OUTPUT);
    digitalWrite(triggerPin, LOW);
    for (uint8_t i = 0; i < sensorCount; i++)
    {
        pinMode(echoPins[i], INPUT);
    }
}

void HcSr05::update()
{
    unsigned long now = millis();
    if (now - lastReadTime < READ_INTERVAL)
        return;

    // send 10us trigger pulse
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    // measure echo duration for the current sensor only
    unsigned long duration = pulseIn(echoPins[currentSensor], HIGH, ECHO_TIMEOUT);

    if (duration > 0)
    {
        distances[currentSensor] = duration * 0.0343 / 2.0;
    }

    // advance to next sensor
    currentSensor = (currentSensor + 1) % sensorCount;
    lastReadTime = now;
}

float HcSr05::getDistance(uint8_t index)
{
    if (index >= sensorCount) return -1.0;
    return distances[index];
}

uint8_t HcSr05::getSensorCount()
{
    return sensorCount;
}

bool HcSr05::allBelow(float threshold)
{
    for (uint8_t i = 0; i < sensorCount; i++)
    {
        if (distances[i] <= 0 || distances[i] > threshold)
            return false;
    }
    return true;
}

float HcSr05::getMinDistance()
{
    float minDist = -1.0;
    for (uint8_t i = 0; i < sensorCount; i++)
    {
        if (distances[i] > 0)
        {
            if (minDist < 0 || distances[i] < minDist)
                minDist = distances[i];
        }
    }
    return minDist;
}
