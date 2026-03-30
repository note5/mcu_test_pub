/**
 * HC-SR05 ultrasonic distance sensor array (shared trigger, multiple echoes)
 *
 * Supports up to MAX_SENSORS sensors sharing one trigger pin, each with its
 * own echo pin. Reads one sensor per interval (sequential cycling), so a
 * full sweep of N sensors takes N * READ_INTERVAL milliseconds.
 */
#ifndef HC_SR05_H
#define HC_SR05_H
#include <Arduino.h>

class HcSr05 {
public:
    static const uint8_t MAX_SENSORS = 5;

    HcSr05(uint8_t trigPin, const uint8_t *echoPins, uint8_t count);
    void begin();                     // configure pins
    void update();                    // call every loop - reads one sensor per interval
    float getDistance(uint8_t index);  // get latest distance for sensor index (cm)
    uint8_t getSensorCount();
    bool allBelow(float threshold);   // true if ALL sensors read <= threshold and > 0
    float getMinDistance();            // smallest valid reading across all sensors

private:
    const uint8_t triggerPin;
    uint8_t echoPins[MAX_SENSORS];
    uint8_t sensorCount;
    float distances[MAX_SENSORS];     // last valid reading per sensor (cm)
    uint8_t currentSensor;            // which sensor to read next
    unsigned long lastReadTime;

    static const unsigned long READ_INTERVAL = 1000;
    static const unsigned long ECHO_TIMEOUT = 30000; // 30ms max (~5m range)
};

#endif
