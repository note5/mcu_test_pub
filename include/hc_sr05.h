/**
 * HC-SR05 ultrasonic distance sensor array (shared trigger, multiple echoes)
 *
 * Hardware: All sensors share a single trigger pin. Each has its own echo pin.
 * Only one echo is read per cycle to avoid crosstalk between sensors.
 *
 * Cycling strategy: Reads one sensor per READ_INTERVAL (1s), then advances
 * to the next. A full sweep of N sensors takes N seconds. This keeps the
 * main loop non-blocking while allowing enough time for echoes to decay
 * before the next trigger pulse.
 *
 * Invalid readings (no echo within timeout) are silently skipped — the
 * previous valid distance is retained until a good reading replaces it.
 * A distance of -1.0 means the sensor has never returned a valid reading.
 */
#ifndef HC_SR05_H
#define HC_SR05_H
#include <Arduino.h>

class HcSr05 {
public:
    static const uint8_t MAX_SENSORS = 5;

    HcSr05(uint8_t trigPin, const uint8_t *echoPins, uint8_t count);
    void begin();
    void update();
    float getDistance(uint8_t index);
    uint8_t getSensorCount();
    bool allBelow(float threshold);
    float getMinDistance();

private:
    const uint8_t triggerPin;
    uint8_t echoPins[MAX_SENSORS];
    uint8_t sensorCount;
    float distances[MAX_SENSORS];     // last valid reading per sensor (cm), -1.0 = no reading yet
    uint8_t currentSensor;            // round-robin index into echoPins[]
    unsigned long lastReadTime;

    // 1s between readings — gives echo time to fully decay and avoids
    // interference between sensors sharing the same trigger line
    static const unsigned long READ_INTERVAL = 1000;
    // 30ms echo timeout caps range at ~5m (speed of sound round-trip)
    static const unsigned long ECHO_TIMEOUT = 30000;
};

#endif
