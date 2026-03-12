#include "ir_array.h"

IrArray::IrArray(uint8_t p0, uint8_t p1, uint8_t p2)
    : pins{p0, p1, p2} {}

void IrArray::begin()
{
    for (uint8_t i = 0; i < NUM_SENSORS; i++)
        pinMode(pins[i], INPUT);
}

bool IrArray::read(uint8_t index)
{
    if (index >= NUM_SENSORS) return false;
    return digitalRead(pins[index]) == LOW;
}

bool IrArray::isFull()
{
    for (uint8_t i = 0; i < NUM_SENSORS; i++)
        if (digitalRead(pins[i]) != LOW) return false;
    return true;
}

void IrArray::printState()
{
    Serial1.print("ir-level:");
    for (uint8_t i = 0; i < NUM_SENSORS; i++) {
        if (i > 0) Serial1.print(",");
        Serial1.print(digitalRead(pins[i]) == LOW ? "1" : "0");
    }
    Serial1.print(",full:");
    Serial1.println(isFull() ? "1" : "0");
}
