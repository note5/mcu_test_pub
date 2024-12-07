#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// Pin definitions
extern const int ENA;  // Enable pin for motor A
extern const int IN1;  // Input 1 pin for motor A
extern const int IN2;  // Input 2 pin for motor A

void motorForward();
void motorBack();
void motorStop();
void processCommand(String command);

#endif