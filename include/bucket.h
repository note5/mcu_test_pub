#ifndef BUCKET_CTRL_H
#define BUCKET_CTRL_H

#include "common.h"
#include "actuators-and-sensors.h"
#include "gyro.h"
#include "timing.h"

namespace BucketControl
{
   String command = "";            // holds the incoming command
    void motor1Ctrl();              //
    void motor2Ctrl();              //
    void motor3Ctrl();              //
    void motor4Ctrl();              //
    void stopAllMotors();           //
    void monitorSwitches();         //
    void logSwitchStates();         //
    void bucketLowerWithLeveling(); //
    void bucketRaiseWithLeveling(); //
    void bucketTip();               //
    //
    uint8_t top_limit_sw_1_val, top_limit_sw_2_val, top_limit_sw_3_val, top_limit_sw_4_val;
    uint8_t bottom_limit_sw_1_val, bottom_limit_sw_2_val, bottom_limit_sw_3_val, bottom_limit_sw_4_val;
    //
    // Command-based control with single state variable
    enum BucketOperation
    {
        NONE,
        LOWERING,
        RAISING,
        TIPPING
    };
    BucketOperation current_operation = NONE;
    int bucket_base_speed = 200;
    unsigned long hold_start_time = 0;
    const unsigned long HOLD_DURATION = 3000; // 3 seconds
    const int TIPPING_SPEED = 180;

    void init()
    {
        SensorsAndActuators::init();
        // enure the motors are all stopped
        stopAllMotors();
    }
    //
    void forever()
    {
        if (Serial1.available() > 0)
        {
            command = Serial1.readStringUntil('\n');
            command.trim();
            debug("incoming serial data: ");
            debugln(command);
        }
        // motor 1 control
        if (command.indexOf("Motor1:") != -1 || motor_1_running == 1)
        {
            if (command.indexOf("Motor1:") != -1)
            {
                String pwm_str = getValue(command, "pwm=");
                motor1.cmd = getValue(command, "cmd=");
                motor1.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 110;
            }
            motor1Ctrl();
        }
        // motor 2 control
        if (command.indexOf("Motor2:") != -1 || motor_2_running == 1)
        {
            if (command.indexOf("Motor2:") != -1)
            {
                String pwm_str = getValue(command, "pwm=");
                motor2.cmd = getValue(command, "cmd=");
                motor2.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 110;
            }
            motor2Ctrl();
        }
        // motor 3 control
        if (command.indexOf("Motor3:") != -1 || motor_3_running == 1)
        {
            if (command.indexOf("Motor3:") != -1)
            {
                String pwm_str = getValue(command, "pwm=");
                motor3.cmd = getValue(command, "cmd=");
                motor3.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 110;
            }
            motor3Ctrl();
        }
        // motor 4 control
        if (command.indexOf("Motor4:") != -1 || motor_4_running == 1)
        {
            if (command.indexOf("Motor4:") != -1)
            {
                String pwm_str = getValue(command, "pwm=");
                motor4.cmd = getValue(command, "cmd=");
                motor4.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 110;
            }
            motor4Ctrl();
        }

        // Bucket serial command handler
        // Bucket: cmd=down,pwm=150
        // Bucket: cmd=up,pwm=150
        // Bucket: cmd=tip
        // Bucket: cmd=stop
        if (command.indexOf("Bucket:") != -1)
        {
            String cmd = getValue(command, "cmd=");
            String pwm_str = getValue(command, "pwm=");
            int speed = pwm_str.length() > 0 ? pwm_str.toInt() : 200;
            speed = constrain(speed, 123, 253); // Clamp to safe range

            if (cmd == "down")
            {
                debugln("Bucket: Lowering with auto-leveling");
                current_operation = LOWERING;
                bucket_base_speed = speed;
                command = "";
            }
            else if (cmd == "up")
            {
                debugln("Bucket: Raising with auto-leveling");
                current_operation = RAISING;
                bucket_base_speed = speed;
                command = "";
            }
            else if (cmd == "tip")
            {
                debugln("Bucket: Starting tip sequence");
                current_operation = TIPPING;
                hold_start_time = 0; // Reset hold timer
                command = "";
            }
            else if (cmd == "stop")
            {
                debugln("Bucket: Stopping all operations");
                current_operation = NONE;
                stopAllMotors();
                command = "";
            }
        }

        // Execute active operation
        switch (current_operation)
        {
        case LOWERING:
            bucketLowerWithLeveling();
            break;
        case RAISING:
            bucketRaiseWithLeveling();
            break;
        case TIPPING:
            bucketTip();
            break;
        case NONE:
            break;
        }

        // monitor limit switches
        monitorSwitches();
      
        if (updateSwitchesLogging.isReady())
        {
            logSwitchStates();
        }
    }
    //  motor1 control
    // Motor1: cmd=clockwise,pwm=150
    // Motor1: cmd=anticlockwise,pwm=150
    // Motor1: cmd=stop
    void motor1Ctrl()
    {
        motor_1_running = true;
        if (motor1.cmd == "clockwise")
        {
            // Forward direction: INAHI PWM, INBLO HIGH, others LOW
            digitalWrite(MOTOR_1_A, LOW);
            digitalWrite(MOTOR_1_PWM_2, LOW);
            digitalWrite(MOTOR_1_B, HIGH);
            analogWrite(MOTOR_1_PWM_1, motor1.speed);
        }
        else if (motor1.cmd == "anticlockwise")
        {
            // Reverse direction: INBHI PWM, INALO HIGH, others LOW
            digitalWrite(MOTOR_1_PWM_1, LOW);
            digitalWrite(MOTOR_1_B, LOW);
            digitalWrite(MOTOR_1_A, HIGH);
            analogWrite(MOTOR_1_PWM_2, motor1.speed);
        }
        else if (motor1.cmd == "stop")
        {
            digitalWrite(MOTOR_1_A, LOW);
            digitalWrite(MOTOR_1_B, LOW);
            analogWrite(MOTOR_1_PWM_1, 0);
            analogWrite(MOTOR_1_PWM_2, 0);
        }
    }
    //  motor2 control
    // Motor2: cmd=clockwise,pwm=252
    // Motor2: cmd=anticlockwise,pwm=252
    // Motor2: cmd=stop
    void motor2Ctrl()
    {
        motor_2_running = true;
        if (motor2.cmd == "clockwise")
        {
            // Forward direction: INAHI PWM, INBLO HIGH, others LOW
            digitalWrite(MOTOR_2_A, LOW);
            digitalWrite(MOTOR_2_PWM_2, LOW);
            digitalWrite(MOTOR_2_B, HIGH);
            analogWrite(MOTOR_2_PWM_1, motor2.speed);
        }
        else if (motor2.cmd == "anticlockwise")
        {
            // Reverse direction: INBHI PWM, INALO HIGH, others LOW
            digitalWrite(MOTOR_2_PWM_1, LOW);
            digitalWrite(MOTOR_2_B, LOW);
            digitalWrite(MOTOR_2_A, HIGH);
            analogWrite(MOTOR_2_PWM_2, motor2.speed);
        }
        else if (motor2.cmd == "stop")
        {
            digitalWrite(MOTOR_2_A, LOW);
            digitalWrite(MOTOR_2_B, LOW);
            analogWrite(MOTOR_2_PWM_1, 0);
            analogWrite(MOTOR_2_PWM_2, 0);
        }
    }
    //  motor3 control
    // Motor3: cmd=clockwise,pwm=126
    // Motor3: cmd=anticlockwise,pwm=252
    // Motor3: cmd=stop
    void motor3Ctrl()
    {
        motor_3_running = true;
        if (motor3.cmd == "clockwise")
        {
            // Forward direction: INAHI PWM, INBLO HIGH, others LOW
            digitalWrite(MOTOR_3_A, LOW);
            digitalWrite(MOTOR_3_PWM_2, LOW);
            digitalWrite(MOTOR_3_B, HIGH);
            analogWrite(MOTOR_3_PWM_1, motor3.speed);
        }
        else if (motor3.cmd == "anticlockwise")
        {
            // Reverse direction: INBHI PWM, INALO HIGH, others LOW
            digitalWrite(MOTOR_3_PWM_1, LOW);
            digitalWrite(MOTOR_3_B, LOW);
            digitalWrite(MOTOR_3_A, HIGH);
            analogWrite(MOTOR_3_PWM_2, motor3.speed);
        }
        else if (motor3.cmd == "stop")
        {
            digitalWrite(MOTOR_3_A, LOW);
            digitalWrite(MOTOR_3_B, LOW);
            analogWrite(MOTOR_3_PWM_1, 0);
            analogWrite(MOTOR_3_PWM_2, 0);
        }
    }
    // motor4 control
    // Motor4: cmd=clockwise,pwm=250
    // Motor4: cmd=anticlockwise,pwm=252
    // Motor4: cmd=stop
    void motor4Ctrl()
    {
        motor_4_running = true;
        if (motor4.cmd == "clockwise")
        {
            // Forward direction: INAHI PWM, INBLO HIGH, others LOW
            digitalWrite(MOTOR_4_A, LOW);
            digitalWrite(MOTOR_4_PWM_2, LOW);
            digitalWrite(MOTOR_4_B, HIGH);
            analogWrite(MOTOR_4_PWM_1, motor4.speed);
        }
        else if (motor4.cmd == "anticlockwise")
        {
            // Reverse direction: INBHI PWM, INALO HIGH, others LOW
            digitalWrite(MOTOR_4_PWM_1, LOW);
            digitalWrite(MOTOR_4_B, LOW);
            digitalWrite(MOTOR_4_A, HIGH);
            analogWrite(MOTOR_4_PWM_2, motor4.speed);
        }
        else if (motor4.cmd == "stop")
        {
            digitalWrite(MOTOR_4_A, LOW);
            digitalWrite(MOTOR_4_B, LOW);
            analogWrite(MOTOR_4_PWM_1, 0);
            analogWrite(MOTOR_4_PWM_2, 0);
        }
    }
    // stop all motors
    void stopAllMotors()
    {
        command = ""; // reset command

        // Reset motor running flags
        motor_1_running = false;
        motor_2_running = false;
        motor_3_running = false;
        motor_4_running = false;

        // Stop all motor hardware
        digitalWrite(MOTOR_1_A, LOW);
        digitalWrite(MOTOR_1_B, LOW);
        analogWrite(MOTOR_1_PWM_1, 0);
        analogWrite(MOTOR_1_PWM_2, 0);
        //
        digitalWrite(MOTOR_2_A, LOW);
        digitalWrite(MOTOR_2_B, LOW);
        analogWrite(MOTOR_2_PWM_1, 0);
        analogWrite(MOTOR_2_PWM_2, 0);
        //
        digitalWrite(MOTOR_3_A, LOW);
        digitalWrite(MOTOR_3_B, LOW);
        analogWrite(MOTOR_3_PWM_1, 0);
        analogWrite(MOTOR_3_PWM_2, 0);
        //
        digitalWrite(MOTOR_4_A, LOW);
        digitalWrite(MOTOR_4_B, LOW);
        analogWrite(MOTOR_4_PWM_1, 0);
        analogWrite(MOTOR_4_PWM_2, 0);
    }
    // Bucket tipping function - tip backward by raising motors 1&2
    void bucketTip()
    {
        // If hold timer not started yet, we're still tipping
        if (hold_start_time == 0)
        {
            // Motors 1&2 move up (clockwise) to raise front and tip backward
            motor1.cmd = "clockwise";
            motor1.speed = TIPPING_SPEED;
            motor2.cmd = "clockwise";
            motor2.speed = TIPPING_SPEED;

            // Motors 3&4 stop and hold position at bottom (act as pivot point)
            motor3.cmd = "stop";
            motor4.cmd = "stop";

            // Execute motor control
            motor1Ctrl();
            motor2Ctrl();
            motor3Ctrl();
            motor4Ctrl();

            // Check if tipped to approximately 80 degrees using gyro
            float pitch = Gyro::getPitch();

            // When pitch reaches ~-55° (front is up, back is down), start holding
            // Negative pitch means back is tilted down (tipping backward to pour)
            if (pitch <= -55.0) // Using -55° threshold
            {
                // Stop motors 1&2
                motor1.cmd = "stop";
                motor2.cmd = "stop";
                motor1Ctrl();
                motor2Ctrl();

                hold_start_time = millis();
                debug("Bucket tipped backward to ");
                debug(pitch);
                debugln(" degrees, holding for 3 seconds");
            }
        }
        else
        {
            // We're in holding phase - check if hold time elapsed
            if (millis() - hold_start_time >= HOLD_DURATION)
            {
                debugln("Hold complete, tip sequence finished");
                current_operation = NONE;
                hold_start_time = 0;
            }
        }
    }

    // Bucket raise with auto-leveling function
    void bucketRaiseWithLeveling()
    {
        // Get current tilt angles from gyro
        float pitch = Gyro::getPitch();
        float roll = Gyro::getRoll();

        // Calculate speed compensation for leveling
        int motor1_speed = bucket_base_speed;
        int motor2_speed = bucket_base_speed;
        int motor3_speed = bucket_base_speed;
        int motor4_speed = bucket_base_speed;

        // Pitch compensation (5 degree threshold)
        if (pitch > 5.0)
        {
            motor1_speed = bucket_base_speed * 0.7;
            motor2_speed = bucket_base_speed * 0.7;
        }
        else if (pitch < -5.0)
        {
            motor3_speed = bucket_base_speed * 0.7;
            motor4_speed = bucket_base_speed * 0.7;
        }

        // Roll compensation (5 degree threshold)
        if (roll > 5.0)
        {
            motor2_speed = motor2_speed * 0.7;
            motor4_speed = motor4_speed * 0.7;
        }
        else if (roll < -5.0)
        {
            motor1_speed = motor1_speed * 0.7;
            motor3_speed = motor3_speed * 0.7;
        }

        // Clamp speeds to safe range
        motor1_speed = constrain(motor1_speed, 60, 253);
        motor2_speed = constrain(motor2_speed, 60, 253);
        motor3_speed = constrain(motor3_speed, 60, 253);
        motor4_speed = constrain(motor4_speed, 60, 253);

        // Set all motors to move up (clockwise)
        motor1.cmd = "clockwise";
        motor1.speed = motor1_speed;
        motor2.cmd = "clockwise";
        motor2.speed = motor2_speed;
        motor3.cmd = "clockwise";
        motor3.speed = motor3_speed;
        motor4.cmd = "clockwise";
        motor4.speed = motor4_speed;

        // Check individual top limit switches and stop respective motors
        if (!top_limit_sw_1_val)
        {
            motor1.cmd = "stop";
            debugln("Motor 1 top limit reached");
        }
        if (!top_limit_sw_2_val)
        {
            motor2.cmd = "stop";
            debugln("Motor 2 top limit reached");
        }
        if (!top_limit_sw_3_val)
        {
            motor3.cmd = "stop";
            debugln("Motor 3 top limit reached");
        }
        if (!top_limit_sw_4_val)
        {
            motor4.cmd = "stop";
            debugln("Motor 4 top limit reached");
        }

        // If all motors reached top, stop raising
        if (!top_limit_sw_1_val && !top_limit_sw_2_val &&
            !top_limit_sw_3_val && !top_limit_sw_4_val)
        {
            current_operation = NONE;
            stopAllMotors();
            debugln("Bucket fully raised");
            return;
        }

        // Execute motor control
        motor1Ctrl();
        motor2Ctrl();
        motor3Ctrl();
        motor4Ctrl();
    }

    // Bucket lower with auto-leveling function
    void bucketLowerWithLeveling()
    {
        // Get current tilt angles from gyro
        float pitch = Gyro::getPitch();
        float roll = Gyro::getRoll();

        // Calculate speed compensation for leveling
        int motor1_speed = bucket_base_speed;
        int motor2_speed = bucket_base_speed;
        int motor3_speed = bucket_base_speed;
        int motor4_speed = bucket_base_speed;

        // Pitch compensation (5 degree threshold)
        if (pitch > 5.0)
        {
            // Front is lower, slow down front motors
            motor1_speed = bucket_base_speed * 0.7;
            motor2_speed = bucket_base_speed * 0.7;
        }
        else if (pitch < -5.0)
        {
            // Back is lower, slow down back motors
            motor3_speed = bucket_base_speed * 0.7;
            motor4_speed = bucket_base_speed * 0.7;
        }

        // Roll compensation (5 degree threshold)
        if (roll > 5.0)
        {
            // Right is lower, slow down right motors
            motor2_speed = motor2_speed * 0.7;
            motor4_speed = motor4_speed * 0.7;
        }
        else if (roll < -5.0)
        {
            // Left is lower, slow down left motors
            motor1_speed = motor1_speed * 0.7;
            motor3_speed = motor3_speed * 0.7;
        }

        // Clamp speeds to safe range
        motor1_speed = constrain(motor1_speed, 60, 253);
        motor2_speed = constrain(motor2_speed, 60, 253);
        motor3_speed = constrain(motor3_speed, 60, 253);
        motor4_speed = constrain(motor4_speed, 60, 253);

        // Set all motors to move down (anticlockwise)
        motor1.cmd = "anticlockwise";
        motor1.speed = motor1_speed;
        motor2.cmd = "anticlockwise";
        motor2.speed = motor2_speed;
        motor3.cmd = "anticlockwise";
        motor3.speed = motor3_speed;
        motor4.cmd = "anticlockwise";
        motor4.speed = motor4_speed;

        // Check individual bottom limit switches and stop respective motors
        if (!bottom_limit_sw_1_val)
        {
            motor1.cmd = "stop";
            debugln("Motor 1 bottom limit reached");
        }
        if (!bottom_limit_sw_2_val)
        {
            motor2.cmd = "stop";
            debugln("Motor 2 bottom limit reached");
        }
        if (!bottom_limit_sw_3_val)
        {
            motor3.cmd = "stop";
            debugln("Motor 3 bottom limit reached");
        }
        if (!bottom_limit_sw_4_val)
        {
            motor4.cmd = "stop";
            debugln("Motor 4 bottom limit reached");
        }

        // If all motors reached bottom, stop lowering
        if (!bottom_limit_sw_1_val && !bottom_limit_sw_2_val &&
            !bottom_limit_sw_3_val && !bottom_limit_sw_4_val)
        {
            debugln("All motors reached bottom - lowering complete");
            current_operation = NONE;
            stopAllMotors();
            return;
        }

        // Execute motor control
        motor1Ctrl();
        motor2Ctrl();
        motor3Ctrl();
        motor4Ctrl();
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
        //     if (!top_limit_sw_1_val || !top_limit_sw_2_val || !top_limit_sw_3_val || !top_limit_sw_4_val)
        //     {
        //         stopAllMotors();
        //     }
    }
  
    //
    void logSwitchStates()
    {
        debug("top 1:");
        debug(top_limit_sw_1_val);
        debug(" top 2:");
        debug(top_limit_sw_2_val);
        debug(" top 3:");
        debug(top_limit_sw_3_val);
        debug(" top 4:");
        debugln(top_limit_sw_4_val);
        debug(" bottom 1:");
        debug(bottom_limit_sw_1_val);
        debug(" bottom 2:");
        debug(bottom_limit_sw_2_val);
        debug(" bottom 3:");
        debug(bottom_limit_sw_3_val);
        debug(" bottom 4:");
        debugln(bottom_limit_sw_4_val);
    }
}

#endif