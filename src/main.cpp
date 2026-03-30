/**
 * Finplus MCU — main entry point
 *
 * ATmega1284P @ 8MHz internal oscillator.
 * Communicates with the Web Serial dashboard (manual.html) over UART1 @ 9600 baud.
 *
 * Subsystems initialised here:
 *   - Conveyor motor (L298N via motor_control.h)
 *   - 5x HC-SR05 ultrasonic sensors for bin fill level
 *   - Platform lift motor with auto-compensation
 *   - 3x magnetic reed door sensors
 *
 * Main loop is fully non-blocking — each subsystem's update() checks its
 * own timing internally. Serial commands are processed when available.
 */
#include <Arduino.h>
#include "motor_control.h"
#include "door_state.h"
#include "platform_control.h"
#include "hc_sr05.h"

// --- HC-SR05 sensor array ---
// 5 sensors arranged in the bin: center + 4 corners.
// All share trigger pin 22; each has a dedicated echo pin.
// Sensors are read one at a time in round-robin (1s apart) to avoid crosstalk.
const uint8_t echoPins[] = {23, 0, 1, 2, 3};  // center, corner1-4
HcSr05 hc_sensors(22, echoPins, 5);

// --- Platform lift motor ---
// Pin order: topLimitSwitch=30, bottomLimitSwitch=31, motorUp=21, motorDown=20
PlatformControl platform(30, 31, 21, 20, &hc_sensors);

// --- Door reed switches ---
// Self-register into DoorState::doors[] for batch reporting
DoorState service_door(24, "service");
DoorState left_door(25, "left");
DoorState right_door(26, "right");

void setup()
{
    // UART1 — hardware serial to the ESP32/Web Serial bridge
    Serial1.begin(9600);

    // Conveyor motor pins (L298N H-bridge)
    pinMode(ENA, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    // Brief startup delay — lets power rails stabilise and serial connect
    delay(1000);
    Serial1.println("Starting ...");
    motorStop();

    hc_sensors.begin();
    platform.begin();

    service_door.begin();
    left_door.begin();
    right_door.begin();
}

/**
 * Auto-report — pushes sensor and platform data to the dashboard every 2s.
 * The dashboard parses these lines to update its UI without polling.
 *
 * Output format examples:
 *   hc-level:12.3,14.5,-1.00,-1.00,-1.00
 *   platform-status:idle,ref:10.00,top:0,bottom:1
 */
void autoReport() {
    static unsigned long lastReport = 0;
    if (millis() - lastReport < 2000) return;
    lastReport = millis();

    Serial1.print("hc-level:");
    for (uint8_t i = 0; i < hc_sensors.getSensorCount(); i++)
    {
        if (i > 0) Serial1.print(",");
        Serial1.print(hc_sensors.getDistance(i));
    }
    Serial1.println();

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
    // --- Non-blocking subsystem updates ---
    // Each checks its own timing internally; safe to call every iteration.
    hc_sensors.update();    // reads one sensor per 1s cycle
    platform.update();      // runs state machine, checks limits
    service_door.update();  // debounces door reed switch
    left_door.update();
    right_door.update();

    autoReport();  // push data to dashboard every 2s

    // --- Serial command processing ---
    // Commands arrive from the Web Serial dashboard (manual.html) as
    // newline-terminated strings over UART1.
    if (Serial1.available() > 0)
    {
        String command = Serial1.readStringUntil('\n');
        command.trim();
        if (command.length() == 0) return;

        if (command == "hc-level")
        {
            // On-demand sensor reading (same format as autoReport)
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
            // Fall through to conveyor motor commands (forward/back/stop)
            processCommand(command);
        }
    }
}