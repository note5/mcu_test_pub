#include "hc_sr05.h"

HcSr05::HcSr05(uint8_t trigPin, uint8_t echoPin)
    : triggerPin(trigPin), echoPin(echoPin),
      latestDistance(-1.0), lastReadTime(0) {}

void HcSr05::begin()
{
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);
    digitalWrite(triggerPin, LOW);
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

    // measure echo duration (us)
    unsigned long duration = pulseIn(echoPin, HIGH, ECHO_TIMEOUT);

    if (duration > 0)
    {
        // speed of sound = 0.0343 cm/us, divide by 2 for round trip
        latestDistance = duration * 0.0343 / 2.0;
    }

    lastReadTime = now;
}

float HcSr05::getDistance()
{
    return latestDistance;
}
