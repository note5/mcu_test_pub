#include "motor_control.h"

// Define the actual pin values
const int ENA = 9;
const int IN1 = 8;
const int IN2 = 7;

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
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    Serial1.println("Stopped");
}

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