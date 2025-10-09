#ifndef HCSRO4_CTRL_H
#define HCSRO4_CTRL_H

#include "common.h"
const int trigPin = 45;
const int echoPin = 44;

namespace Hcsr04
{
    long duration;
    float distance_cm;

    void init()
    {
        pinMode(trigPin, OUTPUT);
        pinMode(echoPin, INPUT);
    }

    void measure()
    {
        // Clear the trig pin
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);

        // Send 10us pulse
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(trigPin, LOW);

        // Read the echo pulse duration
        duration = pulseIn(echoPin, HIGH);

        // Convert to distance (cm)
        distance_cm = duration * 0.0343 / 2;

        // Print result
        // debug("Distance: ");
        // debug(distance_cm);
        // debugln(" cm");
    }
}


#endif