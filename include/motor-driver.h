#ifndef IR2104_DRIVER
#define IR2104_DRIVER

#include "common.h"

// IR2104 H-Bridge Motor Driver Pins

#define MOTOR_1_PWM_A PB4 // PWM Input A
#define MOTOR_1_PWM_B PB5 // PWM Input B
#define MOTOR_1_EN PB8    // Enable pin (digital HIGH/LOW)

#define MOTOR_2_PWM_A PA7 // PWM Input A
#define MOTOR_2_PWM_B PA6 // PWM Input B
#define MOTOR_2_EN PA5    // Enable pin (digital HIGH/LOW)

#define MOTOR_3_PWM_A PB0 // PWM Input A
#define MOTOR_3_PWM_B PB13 // PWM Input B
#define MOTOR_3_EN PB1    // Enable pin (digital HIGH/LOW)

namespace MotorDriver
{

    void motor1Ctrl(); //
    void motor2Ctrl();
    void motor3Ctrl();
    struct Motor1Data
    {
        int speed;
        String cmd;
        bool running;
    };
    struct Motor2Data
    {
        int speed;
        String cmd;
        bool running;
    };
    struct Motor3Data
    {
        int speed;
        String cmd;
        bool running;
    };
    struct Motor4Data
    {
        int speed;
        String cmd;
        bool running;
    };
    // Create instance of the struct
    Motor1Data motor1;
    Motor2Data motor2;
    Motor2Data motor3;
    Motor2Data motor4;
    //    init the driver
    void init()
    {
        // Motor 1
        pinMode(MOTOR_1_PWM_A, OUTPUT);
        pinMode(MOTOR_1_PWM_B, OUTPUT);
        pinMode(MOTOR_1_EN, OUTPUT);
        // Motor 2
        pinMode(MOTOR_2_PWM_A, OUTPUT);
        pinMode(MOTOR_2_PWM_B, OUTPUT);
        pinMode(MOTOR_2_EN, OUTPUT);
        // Motor 3
        pinMode(MOTOR_3_PWM_A, OUTPUT);
        pinMode(MOTOR_3_PWM_B, OUTPUT);
        pinMode(MOTOR_3_EN, OUTPUT);

        // Start with motor 1 disabled
        digitalWrite(MOTOR_1_EN, LOW);
        analogWrite(MOTOR_1_PWM_A, 0);
        analogWrite(MOTOR_1_PWM_B, 0);
        // Start with motor 2 disabled
        digitalWrite(MOTOR_2_EN, LOW);
        analogWrite(MOTOR_2_PWM_A, 0);
        analogWrite(MOTOR_2_PWM_B, 0);
        // Start with motor 3 disabled
        digitalWrite(MOTOR_3_EN, LOW);
        analogWrite(MOTOR_3_PWM_A, 0);
        analogWrite(MOTOR_3_PWM_B, 0);
    }
    // runnier
    void forever()
    {
        // always check serial
        if (SerialDebug.available())
        {
            String motor_command = SerialDebug.readStringUntil('\n');
            motor_command.trim();
            SerialDebug.print("Current cmd: ");
            SerialDebug.println(motor_command);
            // motor 1 control
            if (motor_command.indexOf("Motor1:") != -1)
            {
                String pwm_str = getValue(motor_command, "pwm=");
                motor1.cmd = getValue(motor_command, "cmd=");
                motor1.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 250;
                motor1.running = 1;
            }
            // motor 2 control
            if (motor_command.indexOf("Motor2:") != -1)
            {
                String pwm_str = getValue(motor_command, "pwm=");
                motor2.cmd = getValue(motor_command, "cmd=");
                motor2.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 250;
                motor2.running = 1;
            }
            // motor 3 control
            if (motor_command.indexOf("Motor3:") != -1)
            {
                String pwm_str = getValue(motor_command, "pwm=");
                motor3.cmd = getValue(motor_command, "cmd=");
                motor3.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 250;
                motor3.running = 1;
            }
        }
        // outside serial input
        motor1Ctrl();
        motor2Ctrl();
        motor3Ctrl();
        // motor4Ctrl();
    }
    // Motor 1
    /*
    Motor1: cmd=clockwise,pwm=250
    Motor1: cmd=anticlockwise,pwm=250
    Motor1: cmd=stop
    */
    void motor1Ctrl()
    {
        if (motor1.running == 0)
        {
            return;
        }
        if (motor1.cmd == "clockwise")
        {
            digitalWrite(MOTOR_1_EN, HIGH);
            analogWrite(MOTOR_1_PWM_A, motor1.speed);
            analogWrite(MOTOR_1_PWM_B, 0);
        }
        else if (motor1.cmd == "anticlockwise")
        {
            SerialDebug.println("Running motor2 anticlockwise");
            digitalWrite(MOTOR_1_EN, HIGH);
            analogWrite(MOTOR_1_PWM_A, 0);
            analogWrite(MOTOR_1_PWM_B, motor1.speed);
        }
        else if (motor1.cmd == "stop")
        {
            motor2.running = 0;
            digitalWrite(MOTOR_1_EN, LOW);
            analogWrite(MOTOR_1_PWM_A, 0);
            analogWrite(MOTOR_1_PWM_B, 0);
        }
    }
    // Motor 2
    void motor2Ctrl()
    {
        if (motor2.running == 0)
        {
            return;
        }
        if (motor2.cmd == "clockwise")
        {
            digitalWrite(MOTOR_2_EN, HIGH);
            analogWrite(MOTOR_2_PWM_A, motor2.speed);
            analogWrite(MOTOR_2_PWM_B, 0);
        }
        else if (motor2.cmd == "anticlockwise")
        {

            digitalWrite(MOTOR_2_EN, HIGH);
            analogWrite(MOTOR_2_PWM_A, 0);
            analogWrite(MOTOR_2_PWM_B, motor2.speed);
        }
        else if (motor2.cmd == "stop")
        {
            motor2.running = 0;
            digitalWrite(MOTOR_2_EN, LOW);
            analogWrite(MOTOR_2_PWM_A, 0);
            analogWrite(MOTOR_2_PWM_B, 0);
        }
    }
    // Motor 3
    void motor3Ctrl()
    {
        if (motor3.running == 0)
        {
            return;
        }
        if (motor3.cmd == "clockwise")
        {
            digitalWrite(MOTOR_3_EN, HIGH);
            analogWrite(MOTOR_3_PWM_A, motor3.speed);
            analogWrite(MOTOR_3_PWM_B, 0);
        }
        else if (motor3.cmd == "anticlockwise")
        {

            digitalWrite(MOTOR_3_EN, HIGH);
            analogWrite(MOTOR_3_PWM_A, 0);
            analogWrite(MOTOR_3_PWM_B, motor3.speed);
        }
        else if (motor3.cmd == "stop")
        {
            motor3.running = 0;
            digitalWrite(MOTOR_3_EN, LOW);
            analogWrite(MOTOR_3_PWM_A, 0);
            analogWrite(MOTOR_3_PWM_B, 0);
        }
    }
}

#endif
