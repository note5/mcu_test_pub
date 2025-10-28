#ifndef COMMON_H
#define COMMON_H
#include <Arduino.h>
#include <Wire.h>

extern HardwareSerial SerialDebug;
extern TwoWire gyro_wire;
// checks if print statements are removed or not
#define DEBUG 1
#if DEBUG == 1
#define debug(...) Serial.print(__VA_ARGS__)
#define debugf(...) Serial.printf(__VA_ARGS__)
#define debugln(...) Serial.println(__VA_ARGS__)
#else
#define debug(...)
#define debugln(...)
#define debugf(...)
#endif

//
String getValue(String cmd, String keyword)
{
    int index = cmd.indexOf(keyword);
    if (index == -1)
        return "";
    index += keyword.length();
    int endIndex = cmd.indexOf(',', index);
    if (endIndex == -1)
        endIndex = cmd.indexOf(';', index);
    if (endIndex == -1)
        endIndex = cmd.length();

    return cmd.substring(index, endIndex);
}

#endif

/*
==== USED PINS ====
I2C => PB7, PB6
Ultrasoic = > PA0, PA1
*/ 