#ifndef PLATFORM_CTRL_H
#define PLATFORM_CTRL_H

#include "common.h"
#include "actuators-and-sensors.h"
#include "gyro.h"

namespace PlatformControl
{
    String command = "";      // holds the incoming command
    void motor1Ctrl();        //
    void motor2Ctrl();        //
    void motor3Ctrl();        //
    void motor4Ctrl();        //
    void stopAllMotors();     //
    void monitorSwitches();   //
    void logSwitchStates();   //
    void platformAutoLevel(); //
    void platformMoveUp();    //
    //
    uint8_t top_limit_sw_1_val, top_limit_sw_2_val, top_limit_sw_3_val, top_limit_sw_4_val;
    uint8_t bottom_limit_sw_1_val, bottom_limit_sw_2_val, bottom_limit_sw_3_val, bottom_limit_sw_4_val;
    //
    bool platform_auto_leveling = false;
    bool platform_moving_up = false;
    int platform_base_speed = 254;

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
                motor1.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 250;
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
                motor2.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 250;
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
                motor3.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 250;
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
                motor4.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 250;
            }
            motor4Ctrl();
        }
        // Platform movement with auto leveling
        // Platform: cmd=down,pwm=200
        // Platform: cmd=up,pwm=240
        // Platform: cmd=stop
        if (command.indexOf("Platform:") != -1 || platform_auto_leveling || platform_moving_up)
        {
            if (command.indexOf("Platform:") != -1)
            {
                String cmd = getValue(command, "cmd=");
                String pwm_str = getValue(command, "pwm=");
                int base_speed = pwm_str.length() > 0 ? pwm_str.toInt() : 250;

                // Clamp base_speed: max 253, min calculated to ensure 0.49x gives at least 60
                // min = 60 / 0.49 = 122.45, so minimum is 123
                base_speed = constrain(base_speed, 123, 253);

                if (cmd == "down")
                {
                    platform_auto_leveling = true;
                    platform_base_speed = base_speed;
                    command = ""; // Clear command to prevent re-parsing
                }
                else if (cmd == "up")
                {
                    platform_moving_up = true;
                    platform_base_speed = base_speed;
                    command = ""; // Clear command to prevent re-parsing
                }
                else if (cmd == "stop")
                {
                    debugln("================== Stopping autolevel platform =======================");
                    platform_auto_leveling = false;
                    platform_moving_up = false;
                    stopAllMotors();
                    command = "";
                    // Don't execute leveling functions below, just clear flags and exit this block
                }
            }

            // Only execute leveling functions if flags are still true (not just stopped)
            if (platform_auto_leveling)
            {
                platformAutoLevel();
            }
            else if (platform_moving_up)
            {
                platformMoveUp();
            }
        }
        // monitor limit switches
        monitorSwitches();
        // if (updateSwitchesLogging.isReady())
        // {
        //     logSwitchStates();
        // }
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
    //  motor4 control
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
    // Platform auto-leveling function
    void platformAutoLevel()
    {
        // Get current tilt angles from gyro
        float pitch = Gyro::getPitch(); // Forward(+) / Backward(-) tilt
        float roll = Gyro::getRoll();   // Right(+) / Left(-) tilt

        // Calculate speed compensation based on tilt
        // Motors 1&2 are front, Motors 3&4 are back
        // Motors 1&3 are left, Motors 2&4 are right
      
        int motor1_speed = platform_base_speed;
        int motor2_speed = platform_base_speed;
        int motor3_speed = platform_base_speed;
        int motor4_speed = platform_base_speed;

        // Pitch compensation (forward/back tilt)
        // If pitch > 5°, front is tilted forward (lower), slow down front motors (1&2)
        // If pitch < -5°, back is tilted backward (lower), slow down back motors (3&4)
        if (pitch > 5.0)
        {
            // Front is lower, slow down front motors
            motor1_speed = platform_base_speed * 0.7; // 70% speed
            motor2_speed = platform_base_speed * 0.7;
            // debug("Pitch: Front lower, slowing motors 1&2 - ");
        }
        else if (pitch < -5.0)
        {
            // Back is lower, slow down back motors
            motor3_speed = platform_base_speed * 0.7;
            motor4_speed = platform_base_speed * 0.7;
            // debug("Pitch: Back lower, slowing motors 3&4 - ");
        }

        // Roll compensation (left/right tilt)
        // If roll > 5°, right is tilted (lower), slow down right motors (2&4)
        // If roll < -5°, left is tilted (lower), slow down left motors (1&3)
        if (roll > 5.0)
        {
            // Right is lower, slow down right motors
            motor2_speed = motor2_speed * 0.7;
            motor4_speed = motor4_speed * 0.7;
            // debug("Roll: Right lower, slowing motors 2&4 - ");
        }
        else if (roll < -5.0)
        {
            // Left is lower, slow down left motors
            motor1_speed = motor1_speed * 0.7;
            motor3_speed = motor3_speed * 0.7;
            // debug("Roll: Left lower, slowing motors 1&3 - ");
        }

        // Ensure all motor speeds are at least 60 PWM
        motor1_speed = constrain(motor1_speed, 60, 253);
        motor2_speed = constrain(motor2_speed, 60, 253);
        motor3_speed = constrain(motor3_speed, 60, 253);
        motor4_speed = constrain(motor4_speed, 60, 253);

        // Set motor commands and speeds for downward movement (anticlockwise)
        motor1.cmd = "anticlockwise";
        motor1.speed = motor1_speed;
        motor2.cmd = "anticlockwise";
        motor2.speed = motor2_speed;
        motor3.cmd = "anticlockwise";
        motor3.speed = motor3_speed;
        motor4.cmd = "anticlockwise";
        motor4.speed = motor4_speed;

        // Check individual limit switches and stop respective motors
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

        // If all motors reached bottom, stop auto-leveling
        if (!bottom_limit_sw_1_val && !bottom_limit_sw_2_val &&
            !bottom_limit_sw_3_val && !bottom_limit_sw_4_val)
        {
            platform_auto_leveling = false;
            stopAllMotors();
            debugln("All motors reached bottom - auto-leveling complete");
            return;
        }

        // Execute motor control
        motor1Ctrl();
        motor2Ctrl();
        motor3Ctrl();
        motor4Ctrl();

        // Log tilt info periodically
        if (updateSwitchesLogging.isReady())
        {
            debug("Tilt - Pitch: ");
            debug(pitch);
            debug("° Roll: ");
            debug(roll);
            debug("° Speeds: M1=");
            debug(motor1_speed);
            debug(" M2=");
            debug(motor2_speed);
            debug(" M3=");
            debug(motor3_speed);
            debug(" M4=");
            debugln(motor4_speed);
        }
    }

    // Platform move up function - all motors move up, stop individually at top limit
    void platformMoveUp()
    {
        // Set all motors to move up (clockwise) with base speed
        motor1.cmd = "clockwise";
        motor1.speed = platform_base_speed;
        motor2.cmd = "clockwise";
        motor2.speed = platform_base_speed;
        motor3.cmd = "clockwise";
        motor3.speed = platform_base_speed;
        motor4.cmd = "clockwise";
        motor4.speed = platform_base_speed;

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

        // If all motors reached top, stop moving up
        if (!top_limit_sw_1_val && !top_limit_sw_2_val &&
            !top_limit_sw_3_val && !top_limit_sw_4_val)
        {
            platform_moving_up = false;
            stopAllMotors();
            debugln("All motors reached top - move up complete");
            return;
        }

        // Execute motor control
        motor1Ctrl();
        motor2Ctrl();
        motor3Ctrl();
        motor4Ctrl();
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

        // If auto-leveling or moving up is active, skip global stop - let those functions handle per-motor stops
        if (platform_auto_leveling || platform_moving_up)
        {
            return;
        }

        // stop all motors from moving up
        if (!top_limit_sw_1_val || !top_limit_sw_2_val || !top_limit_sw_3_val || !top_limit_sw_4_val)
        {
            stopAllMotors();
        }
        // stop all motors from moving down
        if (!bottom_limit_sw_1_val || !bottom_limit_sw_2_val || !bottom_limit_sw_3_val || !bottom_limit_sw_4_val)
        {
            stopAllMotors();
        }
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