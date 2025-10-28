#ifndef IR2104_DRIVER
#define IR2104_DRIVER

#include "common.h"

// IR2104 H-Bridge Motor Driver Pins
#define MOTOR_PWM_A PB0 // PWM Input A (TIM3_CH3) - Left half-bridge
#define MOTOR_PWM_B PB1 // PWM Input B (TIM3_CH4) - Right half-bridge
#define MOTOR_EN PC4    // Enable pin (digital HIGH/LOW)

namespace MotorDriver
{
    void init()
    {
        pinMode(MOTOR_PWM_A, OUTPUT);
        pinMode(MOTOR_PWM_B, OUTPUT);
        pinMode(MOTOR_EN, OUTPUT);

        // Start with motor disabled
        digitalWrite(MOTOR_EN, LOW);
        analogWrite(MOTOR_PWM_A, 0);
        analogWrite(MOTOR_PWM_B, 0);
    }

    // Stop the motor
    void stop()
    {
        digitalWrite(MOTOR_EN, LOW);
        analogWrite(MOTOR_PWM_A, 0);
        analogWrite(MOTOR_PWM_B, 0);
    }

    // Brake - short both sides
    void brake()
    {
        digitalWrite(MOTOR_EN, HIGH);
        analogWrite(MOTOR_PWM_A, 0);
        analogWrite(MOTOR_PWM_B, 0);
    }

    // Forward with speed (0-255)
    void forward(uint8_t speed)
    {
        digitalWrite(MOTOR_EN, HIGH);
        analogWrite(MOTOR_PWM_A, speed);
        analogWrite(MOTOR_PWM_B, 0);
    }

    // Reverse with speed (0-255)
    void reverse(uint8_t speed)
    {
        digitalWrite(MOTOR_EN, HIGH);
        analogWrite(MOTOR_PWM_A, 0);
        analogWrite(MOTOR_PWM_B, speed);
    }

}

#endif
