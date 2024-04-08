#include <Arduino.h>
#include <AccelStepper.h>
// holds serial commands
String incoming_serial_command = "";
// commands for crusher
String crusher_command = "";
// checks if there is incoming command from serial
bool in_coming_cmd = false;
// stepper direction pin
#define STEPPER_DIR_PIN 14
// stepper pulse pin
#define STEPPER_PULSE_PIN 12
// instance of stepper lib
AccelStepper stepper(1, STEPPER_PULSE_PIN, STEPPER_DIR_PIN);

// function prototype to control stepper
void runStepperMotor();
void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  // stepper defaults
  stepper.setMaxSpeed(1000); // 100
  stepper.setAcceleration(100);
  // stepper.moveTo(4000);
}
void loop()
{
  // wait for serial input
  if (Serial.available())
  {

    incoming_serial_command = Serial.readStringUntil('\n');
    if (incoming_serial_command)
    {
      Serial.print("Incoming serial command ");
      Serial.println(incoming_serial_command);

      in_coming_cmd = true;
      // ----------------- assign crusher commands -----------------
      if (incoming_serial_command == "ON")
      {
        crusher_command = "ON";
        runStepperMotor();
      }
      if (incoming_serial_command == "OFF")
      {
        crusher_command = "OFF";
      }
    }
  }
}

// run stepper motor
void runStepperMotor()
{
  Serial.print("stepper called ");
  stepper.moveTo(4000);
  stepper.runToPosition();
}