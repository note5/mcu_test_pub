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

namespace BucketControl
{
    //
    void stopAllMotors();
    String getValue(String cmd, String keyword);
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
        //
        pinMode(MOTOR_1_A, OUTPUT);
        pinMode(MOTOR_1_B, OUTPUT);
        pinMode(MOTOR_1_PWM, OUTPUT);
        pinMode(MOTOR_2_A, OUTPUT);
        pinMode(MOTOR_2_B, OUTPUT);
        pinMode(MOTOR_2_PWM, OUTPUT);
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
    // stop all motors
    void stopAllMotors()
    {
        digitalWrite(MOTOR_1_A, LOW);
        digitalWrite(MOTOR_1_B, LOW);
        digitalWrite(MOTOR_2_A, LOW);
        digitalWrite(MOTOR_2_B, LOW);
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

}

#endif