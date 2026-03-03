/**
 * HC-SR05 ultrasonic distance sensor (trigger/echo)
 *
 * Measures distance using a 10us trigger pulse and timing the echo response.
 * Non-blocking: uses a millis()-based interval to take readings every 2s.
 * The trigger pulse (~10us) and pulseIn (~25ms max) run inline.
 */
#ifndef HC_SR05_H
#define HC_SR05_H
#include <Arduino.h>

class HcSr05 {
private:
    const uint8_t triggerPin;
    const uint8_t echoPin;

    float latestDistance;             // last valid reading in cm
    unsigned long lastReadTime;

    static const unsigned long READ_INTERVAL = 1000;
    static const unsigned long ECHO_TIMEOUT = 30000; // 30ms max (~5m range)

public:
    HcSr05(uint8_t trigPin, uint8_t echoPin);
    void begin();                     // configure pins
    void update();                    // call every loop - takes reading at interval
    float getDistance();              // get latest distance in cm
};

#endif
