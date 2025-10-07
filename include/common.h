#ifndef COMMON_H
#define COMMON_H
#include <Arduino.h>

#define BUCKET_CONTROL 0
#define PLATFORM_CONTROL 1
extern uint8_t algo_type;

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