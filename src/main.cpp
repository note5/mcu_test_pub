#include <Arduino.h>
#include "motor_control.h"
#include "door_state.h"
#include "platform_control.h"
#include "hc_sr05.h"

// 5 HC-SR05 sensors: shared trigger pin 22, individual echo pins
// center=23, corner1=0, corner2=1, corner3=2, corner4=3
const uint8_t echoPins[] = {23, 0, 1, 2, 3};
HcSr05 hc_sensors(22, echoPins, 5);
PlatformControl platform(30, 31, 21, 20, &hc_sensors); // topLimit, bottomLimit, motorUp, motorDown

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
    // HC-SR05 sensors setup
    hc_sensors.begin();
    // platform control setup
    platform.begin();
    // set door interrupt
    service_door.begin();
    left_door.begin();
    right_door.begin();
}

// Auto-report sensor levels and platform status
void autoReport() {
    static unsigned long lastReport = 0;
    if (millis() - lastReport < 2000) return;
    lastReport = millis();

    // hc-level:12.3,14.5,-1.00,-1.00,-1.00
    Serial1.print("hc-level:");
    for (uint8_t i = 0; i < hc_sensors.getSensorCount(); i++)
    {
        if (i > 0) Serial1.print(",");
        Serial1.print(hc_sensors.getDistance(i));
    }
    Serial1.println();

    // platform-status:idle,ref:10.00,top:0,bottom:1
    Serial1.print("platform-status:");
    Serial1.print(platform.getStateName());
    Serial1.print(",ref:");
    Serial1.print(platform.getReference());
    Serial1.print(",top:");
    Serial1.print(platform.isTopLimit() ? "1" : "0");
    Serial1.print(",bottom:");
    Serial1.println(platform.isBottomLimit() ? "1" : "0");
}

void loop()
{
    // Update sensors, platform, and door states
    hc_sensors.update();
    platform.update();
    service_door.update();
    left_door.update();
    right_door.update();

    // Auto-report every 2 seconds
    autoReport();

    if (Serial1.available() > 0)
    {
        String command = Serial1.readStringUntil('\n');
        command.trim();
        if (command.length() == 0) return;

        if (command == "hc-level")
        {
            Serial1.print("hc-level:");
            for (uint8_t i = 0; i < hc_sensors.getSensorCount(); i++)
            {
                if (i > 0) Serial1.print(",");
                Serial1.print(hc_sensors.getDistance(i));
            }
            Serial1.println();
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