#ifndef COMMON_H
#define COMMON_H
#include <Arduino.h>
// checks if print statements are removed or not
#define DEBUG 1
#if DEBUG == 1
#define debug(...) Serial1.print(__VA_ARGS__)
#define debugf(...) Serial1.printf(__VA_ARGS__)
#define debugln(...) Serial1.println(__VA_ARGS__)
#else
#define debug(...)
#define debugln(...)
#define debugf(...)
#endif

#endif