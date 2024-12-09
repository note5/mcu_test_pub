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
    
    // Static method to report all door states
    static void getDoorStates();
    
    // Static array to keep track of all instances
    static DoorState* doors[3];
    static uint8_t doorCount;
    
private:
    const uint8_t pin;
    const char* doorName;
    bool triggered;
    bool lastState;
    unsigned long lastDebounceTime;
    static const unsigned long DEBOUNCE_DELAY = 50;
};

#endif