/**
 * Platform level compensation controller
 *
 * As bottles fill the platform, the ultrasonic sensor detects decreasing
 * ullage (distance). When ullage drops by 5cm from the reference, the
 * platform lowers to restore the target distance.
 *
 * Uses a non-blocking state machine: IDLE -> MOVING_DOWN -> IDLE
 * Direction reversals enforce a 1-second dead time to protect the motor.
 * Top and bottom limit switches provide safety bounds.
 */
#ifndef PLATFORM_CONTROL_H
#define PLATFORM_CONTROL_H
#include <Arduino.h>
#include "hc_sr05.h"

class PlatformControl {
private:
    const uint8_t topLimitPin;      // safety limit - platform at highest position
    const uint8_t bottomLimitPin;   // safety limit - platform at lowest position
    const uint8_t motorUpPin;       // HIGH = drive platform up
    const uint8_t motorDownPin;     // HIGH = drive platform down

    HcSr05 *hcSensor;

    float referenceDistance = 10.0;  // target ullage distance (cm) — set this in code
    bool manualMode = false;         // true = manual only, no auto-compensation

    enum State { IDLE, MOVING_DOWN, MOVING_UP, DEAD_TIME };
    State state;
    State pendingState;               // direction to resume after dead time
    unsigned long deadTimeStart;
    float compensationTarget;         // distance to reach before stopping (triggerDist + 5cm)

    // how far (cm) to lower the platform per compensation step
    static constexpr float COMPENSATION_STEP = 5.0;
    // pause (ms) before reversing motor direction
    static constexpr unsigned long DEAD_TIME_MS = 1000;

    float getSensorMinDistance();
    bool allSensorsBelow(float threshold);

public:
    PlatformControl(uint8_t topPin, uint8_t bottomPin,
                    uint8_t upPin, uint8_t downPin,
                    HcSr05 *hc = nullptr);
    void begin();                    // configure pins, stop motor
    void update();                   // call every loop - runs the state machine
    void setReference(float ref);    // manually override reference distance
    float getReference();            // get current reference distance
    bool isTopLimit();               // true if top limit switch is triggered
    bool isBottomLimit();            // true if bottom limit switch is triggered
    void moveUp();                   // drive platform up (respects top limit)
    void moveDown();                 // drive platform down (respects bottom limit)
    void stop();                     // stop platform motor
    const char* getStateName();      // "idle", "moving_down", "moving_up", "dead_time"
};

#endif
