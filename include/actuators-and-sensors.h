#ifndef SENSORS_ACTUATORS_H
#define SENSORS_ACTUATORS_H

#include "common.h"
// Define motor control pins
// Motor 1
#define MOTOR_1_A 52    //
#define MOTOR_1_B 53    //
#define MOTOR_1_PWM_1 2 //
#define MOTOR_1_PWM_2 3 //
// Motor 2
#define MOTOR_2_A 50    //
#define MOTOR_2_B 51    //
#define MOTOR_2_PWM_1 4 // PWM_4
#define MOTOR_2_PWM_2 5 //
// Motor 3
#define MOTOR_3_A 48    //
#define MOTOR_3_B 49    //
#define MOTOR_3_PWM_1 6 //
#define MOTOR_3_PWM_2 7 //
// Motor 4
#define MOTOR_4_A 46    //
#define MOTOR_4_B 47    //
#define MOTOR_4_PWM_1 8 //
#define MOTOR_4_PWM_2 9 //
// Define limit switch input pins
#define TOP_LIMIT_SW_2 30    //
#define TOP_LIMIT_SW_1 31    //
#define TOP_LIMIT_SW_4 32    //
#define TOP_LIMIT_SW_3 33    //
#define BOTTOM_LIMIT_SW_1 34 //
#define BOTTOM_LIMIT_SW_2 35 //
#define BOTTOM_LIMIT_SW_3 36 //
#define BOTTOM_LIMIT_SW_4 37 //

struct Motor1Data
{
    int speed;
    String cmd;
};
// Define struct for motor2
struct Motor2Data
{
    int speed;
    String cmd;
};
// Define struct for motor2
struct Motor3Data
{
    int speed;
    String cmd;
};
// Define struct for motor2
struct Motor4Data
{
    int speed;
    String cmd;
};

// Create instance of the struct
Motor2Data motor1;
Motor2Data motor2;
Motor2Data motor3;
Motor2Data motor4;
//
bool motor_1_running = false;
bool motor_2_running = false;
bool motor_3_running = false;
bool motor_4_running = false;
// 
SmartDelay updateSwitchesLogging(1000); // 100ms custom timer

namespace SensorsAndActuators
{
  //
    void init()
    {
        debugln("Initializing sensors control");

        pinMode(TOP_LIMIT_SW_1, INPUT_PULLUP);
        pinMode(TOP_LIMIT_SW_2, INPUT_PULLUP);
        pinMode(TOP_LIMIT_SW_3, INPUT_PULLUP);
        pinMode(TOP_LIMIT_SW_4, INPUT_PULLUP);
        pinMode(BOTTOM_LIMIT_SW_1, INPUT_PULLUP);
        pinMode(BOTTOM_LIMIT_SW_2, INPUT_PULLUP);
        pinMode(BOTTOM_LIMIT_SW_3, INPUT_PULLUP);
        pinMode(BOTTOM_LIMIT_SW_4, INPUT_PULLUP);
        // Motors
        pinMode(MOTOR_1_A, OUTPUT);
        pinMode(MOTOR_1_B, OUTPUT);
        pinMode(MOTOR_1_PWM_1, OUTPUT);
        pinMode(MOTOR_1_PWM_2, OUTPUT);

        pinMode(MOTOR_2_A, OUTPUT);
        pinMode(MOTOR_2_B, OUTPUT);
        pinMode(MOTOR_2_PWM_1, OUTPUT);
        pinMode(MOTOR_2_PWM_2, OUTPUT);

        pinMode(MOTOR_3_A, OUTPUT);
        pinMode(MOTOR_3_B, OUTPUT);
        pinMode(MOTOR_3_PWM_1, OUTPUT);
        pinMode(MOTOR_3_PWM_2, OUTPUT);

        pinMode(MOTOR_4_A, OUTPUT);
        pinMode(MOTOR_4_B, OUTPUT);
        pinMode(MOTOR_4_PWM_1, OUTPUT);
        pinMode(MOTOR_4_PWM_2, OUTPUT);
       
    }
}

#endif