#include "door_state.h"

// Static self-registration storage — populated by each DoorState constructor
DoorState* DoorState::doors[3] = {nullptr};
uint8_t DoorState::doorCount = 0;

DoorState::DoorState(uint8_t _pin, const char* name) :
    pin(_pin),
    doorName(name),
    triggered(false),
    lastState(LOW),
    lastDebounceTime(0) {
    // Auto-register so getDoorStates() can iterate all doors
    if (doorCount < 3) {
        doors[doorCount++] = this;
    }
}

void DoorState::begin() {
    // No pull-up — external pull-down resistor on the PCB
    pinMode(pin, INPUT);
    lastState = digitalRead(pin);
}

/**
 * Polled debounce — call every loop() iteration.
 *
 * On any state change, the debounce timer resets. Only after the signal
 * has been stable for DEBOUNCE_DELAY (50ms) is it accepted. The
 * "triggered" flag latches on the rising edge (door opened) and stays
 * set until the caller explicitly calls clearTrigger().
 */
void DoorState::update() {
    bool currentState = digitalRead(pin);

    // Reset debounce timer on every state change (bouncing resets the clock)
    if (currentState != lastState) {
        lastDebounceTime = millis();
    }

    // Accept the reading only after it has been stable for 50ms
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        // Latch on rising edge — once triggered, stays true until cleared
        if (currentState == HIGH && !triggered) {
            triggered = true;
        }
    }

    lastState = currentState;
}

bool DoorState::isTriggered() {
    return triggered;
}

// Output format: "door-status:service:1,left:0,right:0\n"
// Parsed by the Web Serial dashboard (manual.html)
void DoorState::getDoorStates() {
    Serial1.print("door-status:");
    for (uint8_t i = 0; i < doorCount; i++) {
        if (i > 0) Serial1.print(",");
        Serial1.print(doors[i]->getName());
        Serial1.print(":");
        Serial1.print(doors[i]->getCurrentState() ? "1" : "0");
    }
    Serial1.println();
}