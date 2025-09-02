# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a bucket control system firmware for an ATmega1284P microcontroller using the Arduino framework. The project controls a 4-motor bucket system with gyroscopic tilt sensing and limit switches for safety.

## Hardware Architecture

- **Target MCU**: ATmega1284P running at 8MHz internal oscillator
- **Communication**: Serial1 (UART) at 9600 baud for command reception and status reporting
- **Sensors**: MPU6050 gyroscope/accelerometer over I2C (Wire library)
- **Motors**: 4 DC motors with PWM speed control and directional pins
- **Safety**: Top and bottom limit switches for each motor (pins 24-31)

## Build Commands

```bash
# Build the project
pio run

# Upload to target (requires USBtinyISP programmer)
pio run -t upload

# Set fuses and bootloader (if needed)
pio run -e fuses_bootloader -t bootloader

# Monitor serial output
pio device monitor --baud 9600
```

## Code Structure

### Core Modules

- **main.cpp**: Main application loop handling serial commands and motor control
- **BucketControl** (bucket-control.h): Motor control functions and limit switch monitoring
- **Gyro** (gyro.h): MPU6050 sensor interface with tilt angle calculations
- **Timing** (timing.h): Non-blocking timer utilities using SmartDelay class
- **common.h**: Debug macros and shared Arduino includes

### Command Protocol

**Individual Motor Control:**
```
Motor1: cmd=clockwise,pwm=150
Motor2: cmd=anticlockwise,pwm=110
Motor3: cmd=stop
Motor4: cmd=clockwise,pwm=200
```

**Coordinated Bucket Movement with Auto-Leveling:**
```
Bucket: cmd=up,pwm=150     # Start continuous upward movement with leveling
Bucket: cmd=down,pwm=120   # Start continuous downward movement with leveling  
Bucket: cmd=stop           # Stop all bucket movement
```

Valid commands: `clockwise`, `anticlockwise`, `stop` (individual motors) | `up`, `down`, `stop` (bucket)
PWM range: 0-255 (default: 110 if not specified)

### Pin Assignments

**Motor Control Pins:**
- Motor 1: A=21, B=20, PWM=14
- Motor 2: A=19, B=18, PWM=15  
- Motor 3: A=0, B=1, PWM=13
- Motor 4: A=2, B=3, PWM=12

**Limit Switch Pins:**
- Top switches: 24-27 (motors 1-4)
- Bottom switches: 28-31 (motors 1-4)

### Safety Features

- All motors automatically stop when any limit switch is triggered
- Limit switches are monitored every 100ms via `monitorSwitchesInterval` timer
- Tilt data is reported every 500ms via `orientationUpdateTimer`
- Bucket auto-leveling mode disables individual motor control to prevent conflicts

### Auto-Leveling System

The `BucketControl::bucketMovement()` function provides continuous gyro-based leveling:

- **Continuous Operation**: Once started, runs continuously in main loop until stopped
- **Real-time Compensation**: Adjusts individual motor speeds based on roll/pitch angles
- **Motor Layout**: Assumes rectangular arrangement (Front-Left, Front-Right, Back-Left, Back-Right)
- **Compensation Factor**: 2.0 degrees per PWM unit (adjustable in `calculateMotorSpeeds()`)
- **Speed Constraints**: Motor speeds constrained between 50-255 PWM
- **Safety Integration**: Respects limit switch states and stops all motors if triggered

## Development Notes

- Debug output is controlled by `DEBUG` flag in common.h (set to 1 for enabled)
- All debug output goes to Serial1, not Serial (USB)
- The project uses header-only implementation for most modules
- Motor structs in main.cpp have naming inconsistencies (all use Motor2Data type)
- Gyroscope readings are converted to roll/pitch angles in degrees