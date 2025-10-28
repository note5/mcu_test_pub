#include <Arduino.h>
#include "gyro.h"
#include "timing.h"
#include "hcsr04.h"

TwoWire gyro_wire(PB7, PB6);
SmartDelay updateOrientation(500);
HardwareSerial SerialDebug(PA3, PA2); // RX, TX

void setup()
{
  Serial.begin(115200);
  SerialDebug.begin(115200);
  // init ultrasonic sensor
  Hcsr04::init();
  // Initialize MPU6050
  gyro_wire.begin();
  Gyro::begin();
}

void loop()
{
  // Non-blocking measurement - just call it every loop, it handles timing
  bool new_distance_data = Hcsr04::measure();

  // Update gyro (non-blocking)
  Gyro::update();

  // Print status periodically
  if (updateOrientation.isReady())
  {
    float current_ullage = Hcsr04::distance_cm;
    float pitch = Gyro::getPitch();
    float roll = Gyro::getRoll();

    SerialDebug.print("Pitch: ");
    SerialDebug.print(pitch);
    SerialDebug.print(" Roll: ");
    SerialDebug.println(roll);
    SerialDebug.print("Distance: ");
    SerialDebug.println(current_ullage);
    SerialDebug.println(Gyro::getTiltString());
  }

  // Optional: React immediately to new distance readings
  // if (new_distance_data) {
  //   SerialDebug.print("New distance: ");
  //   SerialDebug.println(Hcsr04::distance_cm);
  // }
}
