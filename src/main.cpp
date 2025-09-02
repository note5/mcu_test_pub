#include "common.h"
#include "timing.h"
#include "gyro.h"
#include "bucket-control.h"

// Create custom timers if needed
SmartDelay monitorSwitchesInterval(100); // 100ms custom timer

// Define struct for motor2
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

bool motor_1_running = false;
bool motor_2_running = false;
bool motor_3_running = false;
bool motor_4_running = false;

// Continuous bucket movement with auto-leveling
bool bucket_auto_leveling = false;
String bucket_direction = "";
uint8_t bucket_base_speed = 110;

void setup()
{
    Serial1.begin(9600);
    Wire.begin();

    // Initialize MPU6050
    Gyro::begin();
    // init bucket control
    BucketControl::init();
    debugln("MPU6050 initialized...");

    // Initialize motor2 struct
    motor2.speed = 0;
    motor2.cmd = "";
}

void loop()
{
    if (Serial1.available() > 0)
    {
        String command = Serial1.readStringUntil('\n');
        command.trim();
        debug("incoming serial data: ");
        debugln(command);

        // Bucket movement command with auto-leveling (continuous mode)
        if (command.indexOf("Bucket:") != -1)
        {
            String pwm_str = BucketControl::getValue(command, "pwm=");
            String cmd = BucketControl::getValue(command, "cmd=");
            
            if (cmd == "up" || cmd == "down")
            {
                bucket_direction = cmd;
                bucket_base_speed = pwm_str.length() > 0 ? pwm_str.toInt() : 110;
                bucket_auto_leveling = true;
                
                // Stop individual motor flags to avoid conflicts
                motor_1_running = false;
                motor_2_running = false;
                motor_3_running = false;
                motor_4_running = false;
                
                debug("Bucket auto-leveling started: ");
                debug(cmd);
                debug(" at speed ");
                debugln(bucket_base_speed);
            }
            else if (cmd == "stop")
            {
                bucket_auto_leveling = false;
                BucketControl::stopAllMotors();
                debugln("Bucket auto-leveling stopped");
            }
        }
        // Individual motor control (existing functionality)
        else if (command.indexOf("Motor1:") != -1)
        {
            String pwm_str = BucketControl::getValue(command, "pwm=");
            String cmd = BucketControl::getValue(command, "cmd=");
            motor1.cmd = cmd;
            motor1.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 110;
            motor_1_running = true;
           
        }
        if (command.indexOf("Motor2:") != -1)
        {
            String pwm_str = BucketControl::getValue(command, "pwm=");
            String cmd = BucketControl::getValue(command, "cmd=");
            motor2.cmd = cmd;
            motor2.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 110;
            motor_2_running = true;
        
        }
        if (command.indexOf("Motor3:") != -1)
        {
            String pwm_str = BucketControl::getValue(command, "pwm=");
            String cmd = BucketControl::getValue(command, "cmd=");
            motor3.cmd = cmd;
            motor3.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 110;
            motor_3_running = true;
        }
        if (command.indexOf("Motor4:") != -1)
        {
            String pwm_str = BucketControl::getValue(command, "pwm=");
            String cmd = BucketControl::getValue(command, "cmd=");
            motor4.cmd = cmd;
            motor4.speed = pwm_str.length() > 0 ? pwm_str.toInt() : 110;
            motor_4_running = true;
        }
    }

    // Continuous bucket movement with auto-leveling
    if (bucket_auto_leveling)
    {
        // Continuously call bucketMovement for real-time gyro-based leveling
        BucketControl::bucketMovement(bucket_direction, bucket_base_speed);
    }
    // Individual motor commands (only if not in auto-leveling mode)
    else
    {
        if (motor_1_running)
        {
            BucketControl::motor1Ctrl(motor1.cmd, motor1.speed);
        }
        if (motor_2_running)
        {
            BucketControl::motor2Ctrl(motor2.cmd, motor2.speed);
        }
        if (motor_3_running)
        {
            BucketControl::motor3Ctrl(motor3.cmd, motor3.speed);
        }
        if (motor_4_running)
        {
            BucketControl::motor4Ctrl(motor4.cmd, motor4.speed);
        }
    }

    // Non-blocking orientation updates using pre-defined timer
    if (Timing::shouldUpdateOrientation())
    {
        // Update sensor data
        Gyro::update();
        Serial1.print("Tilt: ");
        Serial1.println(Gyro::getTiltString());
    }

    if (monitorSwitchesInterval.isReady())
    {
        BucketControl::monitorSwitches();
    }
}