#include <Arduino.h>
#include "motor_control.h"
#include "door_state.h"
#include "platform_control.h"
#include "hc_sr05.h"

HcSr05 hc_sensor(22, 23); // trigger, echo
PlatformControl platform(27, 28, 21, 20, &hc_sensor); // topLimit, bottomLimit, motorUp, motorDown

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
    delay(1000);
    Serial1.println("Starting ...");
    // Initially stop the motor
    motorStop();
    // HC-SR05 sensor setup
    hc_sensor.begin();
    // platform control setup
    platform.begin();
    // set door interrupt
    service_door.begin();
    left_door.begin();
    right_door.begin();
}

void loop()
{
    // Update sensors, platform, and door states
    hc_sensor.update();
    platform.update();
    service_door.update();
    left_door.update();
    right_door.update();

    if (Serial1.available() > 0)
    {
        String command = Serial1.readStringUntil('\n');
        command.trim();
        if (command.length() == 0) return;

        if (command == "hc-level")
        {
            Serial1.print("hc-level:");
            Serial1.println(hc_sensor.getDistance());
        }
        else if (command == "doors")
        {
            DoorState::getDoorStates();
        }
        else if (command == "platform-up")
        {
            platform.moveUp();
            Serial1.println("platform: moving up");
        }
        else if (command == "platform-down")
        {
            platform.moveDown();
            Serial1.println("platform: moving down");
        }
        else if (command == "platform-stop")
        {
            platform.stop();
            Serial1.println("platform: stopped");
        }
        else if (command == "platform-status")
        {
            Serial1.print("platform-status:");
            Serial1.print(platform.getStateName());
            Serial1.print(",ref:");
            Serial1.print(platform.getReference());
            Serial1.print(",top:");
            Serial1.print(platform.isTopLimit() ? "1" : "0");
            Serial1.print(",bottom:");
            Serial1.println(platform.isBottomLimit() ? "1" : "0");
        }
        else
        {
            processCommand(command);
        }
    }

}