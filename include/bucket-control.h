#ifndef BUCKET_CTRL_H
#define BUCKET_CTRL_H

#include "common.h"

// Define limit switch input pins
#define TOP_LIMIT_SW_1 24    //
#define TOP_LIMIT_SW_2 25    //
#define TOP_LIMIT_SW_3 26    //
#define TOP_LIMIT_SW_4 27    //
#define BOTTOM_LIMIT_SW_1 28 //
#define BOTTOM_LIMIT_SW_2 29 //
#define BOTTOM_LIMIT_SW_3 30 //
#define BOTTOM_LIMIT_SW_4 31 //
// Define motor control pins
// Motor 1
#define MOTOR_1_A 21   //
#define MOTOR_1_B 20   //
#define MOTOR_1_PWM 14 //
// Motor 2
#define MOTOR_2_A 19   //
#define MOTOR_2_B 18   //
#define MOTOR_2_PWM 15 //
// Motor 3
#define MOTOR_3_A 0    //
#define MOTOR_3_B 1    //
#define MOTOR_3_PWM 13 //
// Motor 4
#define MOTOR_4_A 2    //
#define MOTOR_4_B 3    //
#define MOTOR_4_PWM 12 //

namespace BucketControl
{
    //
    void stopAllMotors();
    String getValue(String cmd, String keyword);
    void bucketMovement(String direction, uint8_t baseSpeed);
    
    // Internal function for calculating leveling compensation
    void calculateMotorSpeeds(float roll, float pitch, uint8_t baseSpeed, String direction, 
                             uint8_t &motor1Speed, uint8_t &motor2Speed, uint8_t &motor3Speed, uint8_t &motor4Speed);
    //
    uint8_t top_limit_sw_1_val, top_limit_sw_2_val, top_limit_sw_3_val, top_limit_sw_4_val;
    uint8_t bottom_limit_sw_1_val, bottom_limit_sw_2_val, bottom_limit_sw_3_val, bottom_limit_sw_4_val;
    // init the
    void init()
    {
        pinMode(TOP_LIMIT_SW_1, INPUT);
        pinMode(TOP_LIMIT_SW_2, INPUT);
        pinMode(TOP_LIMIT_SW_3, INPUT);
        pinMode(BOTTOM_LIMIT_SW_1, INPUT);
        pinMode(BOTTOM_LIMIT_SW_2, INPUT);
        pinMode(BOTTOM_LIMIT_SW_3, INPUT);
        pinMode(BOTTOM_LIMIT_SW_4, INPUT);
        // Motors
        pinMode(MOTOR_1_A, OUTPUT);
        pinMode(MOTOR_1_B, OUTPUT);
        pinMode(MOTOR_1_PWM, OUTPUT);

        pinMode(MOTOR_2_A, OUTPUT);
        pinMode(MOTOR_2_B, OUTPUT);
        pinMode(MOTOR_2_PWM, OUTPUT);

        pinMode(MOTOR_3_A, OUTPUT);
        pinMode(MOTOR_3_B, OUTPUT);
        pinMode(MOTOR_3_PWM, OUTPUT);

        pinMode(MOTOR_4_A, OUTPUT);
        pinMode(MOTOR_4_B, OUTPUT);
        pinMode(MOTOR_4_PWM, OUTPUT);
    }
    // monitor limit switches
    void monitorSwitches()
    {
        top_limit_sw_1_val = digitalRead(TOP_LIMIT_SW_1);
        top_limit_sw_2_val = digitalRead(TOP_LIMIT_SW_2);
        top_limit_sw_3_val = digitalRead(TOP_LIMIT_SW_3);
        top_limit_sw_4_val = digitalRead(TOP_LIMIT_SW_4);
        bottom_limit_sw_1_val = digitalRead(BOTTOM_LIMIT_SW_1);
        bottom_limit_sw_2_val = digitalRead(BOTTOM_LIMIT_SW_2);
        bottom_limit_sw_3_val = digitalRead(BOTTOM_LIMIT_SW_3);
        bottom_limit_sw_4_val = digitalRead(BOTTOM_LIMIT_SW_4);

        // stop all motors from moving up
        if (top_limit_sw_1_val || top_limit_sw_2_val || top_limit_sw_3_val || top_limit_sw_4_val)
        {
            stopAllMotors();
        }
        // stop all motors from moving down
        if (bottom_limit_sw_1_val || bottom_limit_sw_2_val || bottom_limit_sw_3_val || bottom_limit_sw_4_val)
        {
            stopAllMotors();
        }
    }
    void logSwitchStates()
    {
        debug("top limit switch 1:");
        debugln(top_limit_sw_1_val);
        debug("top limit switch 2:");
        debugln(top_limit_sw_2_val);
        debug("top limit switch 3:");
        debugln(top_limit_sw_3_val);
        debug("top limit switch 4:");
        debugln(top_limit_sw_4_val);
        debug("bottom limit switch 1:");
        debugln(bottom_limit_sw_1_val);
        debug("bottom limit switch 2:");
        debugln(bottom_limit_sw_2_val);
        debug("bottom limit switch 3:");
        debugln(bottom_limit_sw_3_val);
        debug("bottom limit switch 4:");
        debugln(bottom_limit_sw_4_val);
    }
    // Motor 1 control
    void motor1Ctrl(String cmd, uint8_t speed)
    {

        //
        if (cmd == "clockwise")
        {
            digitalWrite(MOTOR_1_A, LOW);
            digitalWrite(MOTOR_1_B, HIGH);
            analogWrite(MOTOR_1_PWM, speed);
        }
        if (cmd == "anticlockwise")
        {
            digitalWrite(MOTOR_1_A, HIGH);
            digitalWrite(MOTOR_1_B, LOW);
            analogWrite(MOTOR_1_PWM, speed);
        }
        if (cmd == "stop")
        {
            digitalWrite(MOTOR_1_A, LOW);
            digitalWrite(MOTOR_1_B, LOW);
        }
    }
    // Motor 2 control
    void motor2Ctrl(String cmd, uint8_t speed)
    {
        //
        if (cmd == "clockwise")
        {
            digitalWrite(MOTOR_2_A, HIGH);
            digitalWrite(MOTOR_2_B, LOW);
            analogWrite(MOTOR_2_PWM, speed);
        }
        if (cmd == "anticlockwise")
        {
            digitalWrite(MOTOR_2_A, LOW);
            digitalWrite(MOTOR_2_B, HIGH);
            analogWrite(MOTOR_2_PWM, speed);
        }
        if (cmd == "stop")
        {
            digitalWrite(MOTOR_2_A, LOW);
            digitalWrite(MOTOR_2_B, LOW);
        }
    }
    // Motor 3 control
    void motor3Ctrl(String cmd, uint8_t speed)
    {
        //
        if (cmd == "clockwise")
        {
            digitalWrite(MOTOR_3_A, HIGH);
            digitalWrite(MOTOR_3_B, LOW);
            analogWrite(MOTOR_3_PWM, speed);
        }
        if (cmd == "anticlockwise")
        {
            digitalWrite(MOTOR_3_A, LOW);
            digitalWrite(MOTOR_3_B, HIGH);
            analogWrite(MOTOR_3_PWM, speed);
        }
        if (cmd == "stop")
        {
            digitalWrite(MOTOR_3_A, LOW);
            digitalWrite(MOTOR_3_B, LOW);
        }
    }
    // Motor 4 control
    void motor4Ctrl(String cmd, uint8_t speed)
    {
        //
        if (cmd == "clockwise")
        {
            digitalWrite(MOTOR_4_A, HIGH);
            digitalWrite(MOTOR_4_B, LOW);
            analogWrite(MOTOR_4_PWM, speed);
        }
        if (cmd == "anticlockwise")
        {
            digitalWrite(MOTOR_4_A, LOW);
            digitalWrite(MOTOR_4_B, HIGH);
            analogWrite(MOTOR_4_PWM, speed);
        }
        if (cmd == "stop")
        {
            digitalWrite(MOTOR_4_A, LOW);
            digitalWrite(MOTOR_4_B, LOW);
        }
    }
    // stop all motors
    void stopAllMotors()
    {
        digitalWrite(MOTOR_1_A, LOW);
        digitalWrite(MOTOR_1_B, LOW);
        //
        digitalWrite(MOTOR_2_A, LOW);
        digitalWrite(MOTOR_2_B, LOW);
        //
        digitalWrite(MOTOR_3_A, LOW);
        digitalWrite(MOTOR_3_B, LOW);
        //
        digitalWrite(MOTOR_4_A, LOW);
        digitalWrite(MOTOR_4_B, LOW);
    }
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
    
    // Calculate individual motor speeds based on gyro data for leveling
    void calculateMotorSpeeds(float roll, float pitch, uint8_t baseSpeed, String direction, 
                             uint8_t &motor1Speed, uint8_t &motor2Speed, uint8_t &motor3Speed, uint8_t &motor4Speed)
    {
        // Leveling compensation factor (adjust this value to tune responsiveness)
        float compensationFactor = 2.0; // degrees per PWM unit adjustment
        
        // Calculate compensation values based on tilt
        // Roll: positive = tilted right, negative = tilted left
        // Pitch: positive = tilted forward, negative = tilted backward
        int rollCompensation = (int)(roll * compensationFactor);
        int pitchCompensation = (int)(pitch * compensationFactor);
        
        // Assume motor layout:
        // Motor1: Front-Left,  Motor2: Front-Right
        // Motor3: Back-Left,   Motor4: Back-Right
        
        // Apply compensation to each motor
        int motor1Adjust = -pitchCompensation - rollCompensation; // Front-left
        int motor2Adjust = -pitchCompensation + rollCompensation; // Front-right  
        int motor3Adjust = pitchCompensation - rollCompensation;  // Back-left
        int motor4Adjust = pitchCompensation + rollCompensation;  // Back-right
        
        // Calculate final speeds with constraints
        motor1Speed = constrain(baseSpeed + motor1Adjust, 50, 255);
        motor2Speed = constrain(baseSpeed + motor2Adjust, 50, 255);
        motor3Speed = constrain(baseSpeed + motor3Adjust, 50, 255);
        motor4Speed = constrain(baseSpeed + motor4Adjust, 50, 255);
    }
    
    // Main bucket movement function with gyro-based leveling
    void bucketMovement(String direction, uint8_t baseSpeed)
    {
        // Safety check - stop if any limit switch is active
        if (top_limit_sw_1_val || top_limit_sw_2_val || top_limit_sw_3_val || top_limit_sw_4_val ||
            bottom_limit_sw_1_val || bottom_limit_sw_2_val || bottom_limit_sw_3_val || bottom_limit_sw_4_val)
        {
            stopAllMotors();
            debug("Movement stopped - limit switch active");
            return;
        }
        
        // Get current gyro readings
        float currentRoll = Gyro::getRoll();
        float currentPitch = Gyro::getPitch();
        
        // Calculate individual motor speeds for leveling
        uint8_t motor1Speed, motor2Speed, motor3Speed, motor4Speed;
        calculateMotorSpeeds(currentRoll, currentPitch, baseSpeed, direction, 
                           motor1Speed, motor2Speed, motor3Speed, motor4Speed);
        
        // Determine motor direction based on command
        String motorCmd;
        if (direction == "up")
        {
            motorCmd = "clockwise"; // Adjust based on your motor wiring
        }
        else if (direction == "down")
        {
            motorCmd = "anticlockwise"; // Adjust based on your motor wiring
        }
        else if (direction == "stop")
        {
            stopAllMotors();
            return;
        }
        else
        {
            debug("Invalid direction: ");
            debugln(direction);
            return;
        }
        
        // Execute coordinated movement with leveling compensation
        motor1Ctrl(motorCmd, motor1Speed);
        motor2Ctrl(motorCmd, motor2Speed);
        motor3Ctrl(motorCmd, motor3Speed);
        motor4Ctrl(motorCmd, motor4Speed);
        
        // Debug output
        debug("Bucket movement: ");
        debug(direction);
        debug(" | Roll: ");
        debug(currentRoll);
        debug(" | Pitch: ");
        debug(currentPitch);
        debug(" | Speeds: ");
        debug(motor1Speed);
        debug(",");
        debug(motor2Speed);
        debug(",");
        debug(motor3Speed);
        debug(",");
        debugln(motor4Speed);
    }

}

#endif