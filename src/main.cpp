#include <Arduino.h>
#include "motor_control.h"
#include "level_sensor.h"
#include "door_state.h"

I2cLevelSensor level_sensor;
//
DoorState service_door(24, "service");
DoorState left_door(25, "left");
DoorState right_door(26, "right");

void setup()
{
    Serial1.begin(9600);
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    // Initially stop the motor
    motorStop();
    // Level sensor setup
    if (!level_sensor.begin())
    {
        Serial1.println("Sensor not found!");
    }
    // set door interrupt
    service_door.begin();
    left_door.begin();
    right_door.begin();
}

void loop()
{
    // Update sensor and door states
    level_sensor.update();
    service_door.update();
    left_door.update();
    right_door.update();

    if (Serial1.available() > 0)
    {
        String command = Serial1.readStringUntil('\n');
        command.trim();
        // get pin level
        if (command == "level")
        {
            Serial1.print("level:");
            Serial1.println(level_sensor.getDistance());
            return;
        }
        if (command == "doors")
        {
            DoorState::getDoorStates();
            return;
        }
        else
        {
            processCommand(command); // Handle motor commands
            return;
        }
    }
}