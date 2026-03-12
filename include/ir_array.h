/**
 * IR sensor array for bin level validation
 *
 * 3 digital IR sensors at the top of the bin to check even spread.
 * Each sensor reads LOW (0) when an object is in front of it.
 * All sensors reading 0 = bin is truly full and evenly spread.
 */
#ifndef IR_ARRAY_H
#define IR_ARRAY_H
#include <Arduino.h>

class IrArray {
private:
    static const uint8_t NUM_SENSORS = 3;
    uint8_t pins[NUM_SENSORS];

public:
    IrArray(uint8_t p0, uint8_t p1, uint8_t p2);
    void begin();
    bool read(uint8_t index);
    bool isFull();
    void printState();
};

#endif
