#include "door_state.h"

// Initialize static members
DoorState* DoorState::doors[3] = {nullptr};
uint8_t DoorState::doorCount = 0;

DoorState::DoorState(uint8_t _pin, const char* name) : 
    pin(_pin), 
    doorName(name),
    triggered(false), 
    lastState(LOW),
    lastDebounceTime(0) {
    if (doorCount < 3) {
        doors[doorCount++] = this;
    }
}

void DoorState::begin() {
    pinMode(pin, INPUT);
    lastState = digitalRead(pin);
}
//update door state showing Ken
void DoorState::update() {
    bool currentState = digitalRead(pin);
    
    if (currentState != lastState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        if (currentState == HIGH && !triggered) {
            triggered = true;
        }
    }
    
    lastState = currentState;
}

bool DoorState::isTriggered() {
    return triggered;
}

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