#include "motor_control.h"

// L298N wiring — see pin map in motor_control.h
const int ENA = 9;    // PWM-capable pin for speed control
const int IN1 = 19;   // direction pin A
const int IN2 = 18;   // direction pin B

// All motor functions run at full speed (255). PWM duty cycle can be
// reduced here if slower conveyor speed is needed.

void motorForward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 255);
    Serial1.println("Moving forward");
}

void motorBack() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, 255);
    Serial1.println("Moving backward");
}

void motorStop() {
    // Both direction pins LOW + ENA 0 = brake mode on L298N
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    Serial1.println("Stopped");
}

// Fallback command handler for conveyor motor commands.
// Called from main.cpp when no other command matches.
void processCommand(String command) {
    if (command == "forward") {
        motorForward();
    }
    else if (command == "back") {
        motorBack();
    }
    else if (command == "stop") {
        motorStop();
    }
    else {
        Serial1.println("Unknown command. Use 'forward', 'back', or 'stop'.");
    }
}