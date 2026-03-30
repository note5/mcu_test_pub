/**
 * Magnetic reed switch door sensor with software debounce
 *
 * Each door (service, left, right) has a reed switch that reads HIGH
 * when the door is open. The class debounces the signal (50ms) and
 * latches a "triggered" flag on the LOW->HIGH transition, which can
 * be polled and cleared by the caller.
 *
 * Self-registering: each DoorState constructor appends itself to the
 * static doors[] array, so getDoorStates() can iterate all instances
 * without the caller maintaining a separate list. Limited to 3 doors.
 */
#ifndef DOOR_STATE_H
#define DOOR_STATE_H
#include <Arduino.h>

class DoorState {
public:
    DoorState(uint8_t pin, const char* name);
    void begin();
    bool isTriggered();
    void clearTrigger() { triggered = false; }
    void update();
    bool getCurrentState() { return lastState; }
    const char* getName() { return doorName; }

    // Sends comma-separated door states over Serial1 for the dashboard
    static void getDoorStates();

    // Self-registration — constructor adds each instance here (max 3)
    static DoorState* doors[3];
    static uint8_t doorCount;

private:
    const uint8_t pin;
    const char* doorName;
    bool triggered;             // latched on rising edge, caller must clearTrigger()
    bool lastState;             // previous raw reading for edge detection
    unsigned long lastDebounceTime;
    // 50ms debounce — reed switches bounce for ~20-30ms typically
    static const unsigned long DEBOUNCE_DELAY = 50;
};

#endif