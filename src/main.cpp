#include <Arduino.h>
#include "motor_control.h"
#include "level_sensor.h"

I2cLevelSensor level_sensor;

void setup()
{
    Serial1.begin(9600);
    Serial.begin(9600);

    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    // Initially stop the motor
    motorStop();
    // Level sensor setup
    if (!level_sensor.begin())
    {
        Serial1.println("Sensor not found!");
        while (1)
            ;
    }
}

void loop()
{
    float distance = level_sensor.readDistanceInCM();
    Serial1.print("Distance: ");
    Serial1.print(distance);
    Serial1.println(" cm");
    delay(100);
    if (Serial1.available() > 0)
    {
        String command = Serial1.readStringUntil('\n');
        command.trim();

        if (command == "level")
        {
            float avgDistance = level_sensor.readAverageDistance(10, 100);
            Serial1.print("Average Distance: ");
            Serial1.print(avgDistance);
            Serial1.println(" cm");
        }
        else
        {
            processCommand(command); // Handle motor commands
        }
    }
}