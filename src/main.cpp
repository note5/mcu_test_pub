#include "common.h"
#include "timing.h"
#include "gyro.h"
#include "bucket.h"
#include "hcsr04.h"
#include "lowering-platform.h"

// Create custom timers if needed
SmartDelay updateLevel(1000);       // 100ms custom timer
uint8_t algo_type = BUCKET_CONTROL; //  Bucket control
// uint8_t algo_type = PLATFORM_CONTROL; // Platform control
void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  Wire.begin();
  // use bucket control
  if (algo_type == BUCKET_CONTROL)
  {
    debugln("Running bucket control");
    BucketControl::init();
  }
  // use platform control
  if (algo_type == PLATFORM_CONTROL)
  {
    debugln("Running platform control");
    PlatformControl::init();
  }
  // init ultrasonic sensor
  Hcsr04::init();
  // Initialize MPU6050
  Gyro::begin();
}

void loop()
{
  // Non-blocking orientation updates using pre-defined timer
  if (Timing::shouldUpdateOrientation())
  {
    // Update sensor data
    Gyro::update();
  }
  //
  if (algo_type == BUCKET_CONTROL)
  {
    BucketControl::forever();
  }
  //
  if (algo_type == PLATFORM_CONTROL)
  {
    PlatformControl::forever();
  }
  if (updateLevel.isReady())
  {
    float pitch = Gyro::getPitch();
    float roll = Gyro::getRoll();
    debug("Pitch: ");
    debug(pitch);
    debug(" Roll: ");
    debugln(roll);
  }
}