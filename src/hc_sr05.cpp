#include "hc_sr05.h"

HcSr05::HcSr05(uint8_t trigPin, const uint8_t *echoes, uint8_t count)
    : triggerPin(trigPin), sensorCount(min(count, MAX_SENSORS)),
      currentSensor(0), lastReadTime(0)
{
    for (uint8_t i = 0; i < sensorCount; i++)
    {
        echoPins[i] = echoes[i];
        distances[i] = -1.0;   // -1 signals "no valid reading yet"
    }
}

void HcSr05::begin()
{
    pinMode(triggerPin, OUTPUT);
    digitalWrite(triggerPin, LOW);  // idle low — trigger pulse is active-high
    for (uint8_t i = 0; i < sensorCount; i++)
    {
        pinMode(echoPins[i], INPUT);
    }
}

/**
 * Non-blocking sensor read — call every loop() iteration.
 *
 * Each call reads exactly one sensor (the one at currentSensor), then
 * advances the index. This round-robin approach avoids crosstalk: only
 * one echo pin is listened to per trigger pulse, even though all sensors
 * fire simultaneously from the shared trigger.
 *
 * Note: pulseIn() is blocking (up to 30ms worst case), which is acceptable
 * on this 8MHz ATmega — the rest of the loop has no tight timing needs.
 */
void HcSr05::update()
{
    unsigned long now = millis();
    if (now - lastReadTime < READ_INTERVAL)
        return;

    // HC-SR05 trigger protocol: 2us LOW settle, then 10us HIGH pulse
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);

    // Only listen on the current sensor's echo pin — ignore all others
    unsigned long duration = pulseIn(echoPins[currentSensor], HIGH, ECHO_TIMEOUT);

    if (duration > 0)
    {
        // Speed of sound = 343 m/s = 0.0343 cm/us, divide by 2 for round-trip
        distances[currentSensor] = duration * 0.0343 / 2.0;
    }
    // On timeout (duration == 0), keep the previous reading — avoids glitchy
    // -1 values when a single pulse is missed

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

/**
 * Returns true only if every sensor has a valid reading (> 0) that is
 * at or below the threshold. Used by PlatformControl to confirm the
 * bin is uniformly full before triggering compensation.
 */
bool HcSr05::allBelow(float threshold)
{
    for (uint8_t i = 0; i < sensorCount; i++)
    {
        if (distances[i] <= 0 || distances[i] > threshold)
            return false;
    }
    return true;
}

/**
 * Returns the smallest valid distance across all sensors, or -1.0 if
 * none have valid readings. The minimum represents the highest point
 * of bin contents (closest to the sensor).
 */
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
