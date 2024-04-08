#include <Arduino.h>
#include <AccelStepper.h>
// IR sensor definitions
#define LED_PIN 13
#define IR_SENSOR_1 24
#define IR_SENSOR_2 25
#define IR_SENSOR_3 26
#define IR_SENSOR_4 27
#define IR_SENSOR_5 28
#define IR_SENSOR_6 29
#define IR_SENSOR_7 30
#define IR_SENSOR_8 31
uint8_t ir_sensors[8] = {IR_SENSOR_1, IR_SENSOR_2, IR_SENSOR_3, IR_SENSOR_4, IR_SENSOR_5, IR_SENSOR_6, IR_SENSOR_7, IR_SENSOR_8};
// PWM pin definitions
#define PWM_1 12
#define PWM_2 13
#define PWM_3 14
#define PWM_4 15
// Digital output pins
#define OUTPUT_1 23 // used to control the door relay
#define OUTPUT_2 22
#define OUTPUT_3 21
#define OUTPUT_4 20
#define OUTPUT_5 19
#define OUTPUT_6 18
#define OUTPUT_7 17
#define OUTPUT_8 16 // used to control the crusher relay
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
// crusher relay pin
#define CRUSHER_CONTROL_PIN OUTPUT_8
unsigned long crusher_start_time;
unsigned long crusher_running_duration = 10000; // 10 run the crusher for 10 seconds
// instance of stepper lib
AccelStepper stepper(1, STEPPER_PULSE_PIN, STEPPER_DIR_PIN);

// function prototype to control stepper
void runStepperMotor();
//
void stopCrusher();
void runCrusher();
void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  // stepper defaults
  stepper.setMaxSpeed(1500); // 100
  stepper.setAcceleration(200);
  // set crusher relay control pin to output
  pinMode(CRUSHER_CONTROL_PIN, OUTPUT);
  //
  crusher_start_time = millis();
  // make all the ir sensor pins to input
  for (int i = 0; i < 8; i++)
  {
    pinMode(ir_sensors[i], INPUT);
  }
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
        runCrusher(); // run crusher
      }
      if (incoming_serial_command == "OFF")
      {
        crusher_command = "OFF";
        stopCrusher();
      }
    }
  }

  // run the crusher for the duration set
  if (millis() - crusher_start_time < crusher_running_duration && crusher_command == "ON")
  {
    runCrusher();
  }
  else
  {
    crusher_start_time = millis();
    stopCrusher();
  }
}

// run stepper motor
void runStepperMotor()
{
  Serial.print("stepper called ");
  stepper.moveTo(4000);
  stepper.runToPosition();
  stepper.setCurrentPosition(0);
}

// run crusher
void runCrusher()
{
  Serial.println("Crusher is starting ....");
  digitalWrite(CRUSHER_CONTROL_PIN, HIGH);
  delay(100);
}

// stop crusher
void stopCrusher()
{
  Serial.println("Crusher is stopping ....");
  digitalWrite(CRUSHER_CONTROL_PIN, LOW);
  delay(100);
  in_coming_cmd = false;
  crusher_command = "";
}