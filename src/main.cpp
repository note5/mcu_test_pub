#include <Arduino.h>
#include "motor_control.h"

void setup() {
    Serial1.begin(9600);
    
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    
    motorStop();  // Initially stop the motor
}

void loop() {
    if (Serial1.available() > 0) {
        String command = Serial1.readStringUntil('\n');
        command.trim();
        processCommand(command);
    }
}