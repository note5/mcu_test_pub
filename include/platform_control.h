/**
 * Platform level compensation controller
 *
 * Context: A waste bin sits on a motorised platform. As bottles accumulate,
 * the fill level rises and the ultrasonic ullage distance shrinks. This
 * controller lowers the platform in fixed 5cm steps so the opening stays
 * at a usable height for the user.
 *
 * State machine (non-blocking, driven by update() each loop iteration):
 *
 *   IDLE ──(all sensors <= referenceDistance)──> MOVING_DOWN
 *   MOVING_DOWN ──(ullage restored by 5cm)──> IDLE
 *   Any state ──(direction reversal requested)──> DEAD_TIME ──(1s elapsed)──> target state
 *
 * Safety:
 *   - Limit switches on top/bottom hard-stop the motor regardless of state
 *   - Dead time prevents H-bridge shoot-through on rapid direction changes
 *   - manualMode disables auto-compensation for bench testing
 *
 * Compile-time tuning: change referenceDistance and manualMode in the
 * header rather than at runtime (no serial command to set them).
 */
#ifndef PLATFORM_CONTROL_H
#define PLATFORM_CONTROL_H
#include <Arduino.h>
#include "hc_sr05.h"

class PlatformControl {
private:
    // Limit switch pins — wired to pull LOW normally, HIGH when triggered
    const uint8_t topLimitPin;
    const uint8_t bottomLimitPin;
    // Motor relay/driver pins — directly drive an H-bridge or relay pair
    const uint8_t motorUpPin;
    const uint8_t motorDownPin;

    HcSr05 *hcSensor;

    // Compile-time settings — edit these values directly, not via serial
    float referenceDistance = 10.0;  // ullage threshold (cm) that triggers compensation
    bool manualMode = false;         // true = serial commands only, no auto-lowering

    enum State { IDLE, MOVING_DOWN, MOVING_UP, DEAD_TIME };
    State state;
    State pendingState;               // which direction to resume after dead time elapses
    unsigned long deadTimeStart;
    float compensationTarget;         // ullage reading (cm) at which to stop lowering

    // Platform drops 5cm per compensation event to match typical bottle height
    static constexpr float COMPENSATION_STEP = 5.0;
    // 1s motor-off pause before reversing — protects H-bridge from shoot-through
    static constexpr unsigned long DEAD_TIME_MS = 1000;
    // Only re-evaluate compensation every 10s to avoid reacting to transient readings
    static constexpr unsigned long CHECK_INTERVAL_MS = 10000;

    unsigned long lastCheckTime;

    float getSensorMinDistance();
    bool allSensorsBelow(float threshold);

public:
    PlatformControl(uint8_t topPin, uint8_t bottomPin,
                    uint8_t upPin, uint8_t downPin,
                    HcSr05 *hc = nullptr);
    void begin();
    void update();
    void setReference(float ref);
    float getReference();
    bool isTopLimit();
    bool isBottomLimit();
    void moveUp();
    void moveDown();
    void stop();
    const char* getStateName();
};

#endif
