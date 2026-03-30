/**
 * Conveyor belt motor control via L298N H-bridge
 *
 * This is the conveyor/feeder motor, separate from the platform lift motor
 * (which is controlled by PlatformControl). Drives the belt that moves
 * bottles into the bin.
 *
 * Wiring: ATmega1284P -> L298N driver board
 *   ENA (pin 9)  — PWM speed control (always 255 = full speed for now)
 *   IN1 (pin 19) — direction input 1
 *   IN2 (pin 18) — direction input 2
 */
#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

extern const int ENA;   // PWM enable — pin 9
extern const int IN1;   // direction A — pin 19
extern const int IN2;   // direction B — pin 18

void motorForward();
void motorBack();
void motorStop();
void processCommand(String command);

#endif