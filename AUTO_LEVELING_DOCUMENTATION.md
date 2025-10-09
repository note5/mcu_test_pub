# Platform Auto-Leveling System Documentation

## Overview
This document describes the enhanced auto-leveling system for a platform suspended by 4 pulleys, each controlled by an independent motor at each corner. The system uses a gyroscope (MPU6050) to detect platform tilt, automatically adjusts motor speeds to maintain level descent, and integrates an ultrasonic sensor (HC-SR04) for ullage-based automatic stops during material filling operations.

---

## System Architecture

### Hardware Components
- **4 DC Motors**: Each controls one corner pulley (front-left, front-right, back-left, back-right)
- **8 Limit Switches**: Top and bottom switches for each of the 4 motors
- **MPU6050 Gyroscope**: Measures platform tilt (pitch and roll)
- **HC-SR04 Ultrasonic Sensor**: Measures distance (ullage) from sensor to platform/material surface
- **Arduino Mega 2560**: Main controller
- **Serial Communication**: Command input via Serial1 (9600 baud)

### Motor-Corner Mapping
```
        FRONT
    [M1]      [M2]

LEFT              RIGHT

    [M3]      [M4]
        BACK
```

- **Motor 1**: Front-Left
- **Motor 2**: Front-Right
- **Motor 3**: Back-Left
- **Motor 4**: Back-Right

### Gyroscope Orientation
- **Pitch**: Forward (+) / Backward (-) tilt
  - Positive pitch → Front tilted down (lower)
  - Negative pitch → Back tilted down (lower)
- **Roll**: Right (+) / Left (-) tilt
  - Positive roll → Right side tilted down (lower)
  - Negative roll → Left side tilted down (lower)

### Ultrasonic Sensor Setup
- **Trigger Pin**: 45
- **Echo Pin**: 44
- **Measurement**: Distance in centimeters from sensor to platform/material surface
- **Ullage**: The distance (air gap) between sensor and material surface
- **Threshold**: 15cm (configurable) - platform operates when ullage is below this value
- **Valid Range**: 2-200cm (readings outside this range are filtered as sensor errors)
- **Update Frequency**: Measured once per main loop (before platform control executes)

---

## Implementation Files

### Modified Files
1. **[src/main.cpp](src/main.cpp)**
   - Lines 37-41: Enabled gyro sensor updates every 500ms

2. **[include/lowering-platform.h](include/lowering-platform.h)**
   - Lines 6-7: Added `#include "gyro.h"` and `#include "hcsr04.h"`
   - Lines 11-14: Ullage tracking variables (last_ullage_cm, ullage_threshold_cm, sensor validation limits)
   - Lines 16-22: State machine enum (TOP, LOWERING, BOTTOM, RAISING)
   - Lines 31-33: Function declarations (platformAutoLevel, platformMoveUp, checkUllageAndAutoStart)
   - Lines 49-50: Auto-start ullage check called at beginning of forever loop
   - Lines 104-138: Platform command parser (down, up, stop) with enum-based state management
   - Lines 110-112: PWM range validation (123-253)
   - Lines 140-156: State machine switch statement
   - Lines 290-420: Auto-leveling algorithm with continuous ullage tracking and PWM clamping
   - Lines 296-304: Sensor validation (uses last valid reading if current is invalid)
   - Lines 349-374: Ullage-based stop logic (stops if ullage > threshold, continues if ≤ threshold)
   - Lines 422-464: Platform move up implementation with per-motor limit control
   - Lines 525-541: Modified switch monitoring for per-motor control during operations
   - Lines 568-591: Auto-start function with sensor validation

3. **[include/hcsr04.h](include/hcsr04.h)**
   - Complete ultrasonic sensor driver implementation

---

## Enhanced State Machine

### State Overview

The platform control system uses an enum-based state machine for clear, predictable operation:

```cpp
enum PlatformOperation {
    TOP,              // All top limit switches active, or ullage > threshold
    LOWERING,         // Auto-leveling descent tracking material level
    BOTTOM,           // All bottom limit switches active, fully lowered
    RAISING           // Moving up to top
};
```

### State Transitions

```
System Start / ullage > 15cm
        ↓
    ┌───────┐
    │  TOP  │◄─────────────────────────┐
    │       │                          │
    └───────┘                          │
        │                              │
        │ Auto-start:                  │
        │ ullage < 15cm                │
        │ (material close)             │
        ▼                              │
    ┌──────────┐                       │
    │ LOWERING │                       │
    │          │                       │
    └──────────┘                       │
        │                              │
        │ Continuous operation:        │
        │ • ullage < 15cm → Continue   │
        │ • ullage > 15cm → Stop & TOP │
        │                              │
        │                              │
        │ All bottom                   │
        │ limits = 0                   │
        ▼                              │
    ┌────────┐                         │
    │ BOTTOM │                         │
    │        │                         │
    └────────┘                         │
        │                              │
        │ cmd=up                       │
        ▼                              │
    ┌─────────┐                        │
    │ RAISING │                        │
    │         │                        │
    └─────────┘                        │
        │                              │
        │ All top limits = 0           │
        └──────────────────────────────┘
```

### State Descriptions

#### **TOP** (Idle/Ready)
- **Entry**: System startup, ullage > 15cm, or after stop command
- **Purpose**: Ready state, auto-start monitoring active
- **Motor Control**: All motors stopped
- **Auto-Start Check**: Continuously monitors ullage
  - If ullage < 15cm → Automatically transitions to LOWERING
  - Material is close to sensor, needs space
- **Transitions**:
  - ullage < 15cm (auto) → LOWERING
  - `cmd=down` (manual) → LOWERING
  - Manual motor commands allowed

#### **LOWERING** (Auto-Leveling Descent Tracking Material)
- **Entry**: From TOP (auto-start or manual command)
- **Purpose**: Lower platform with auto-leveling, track material level
- **Motor Control**:
  - All motors move down (anticlockwise)
  - Speed compensated based on tilt (±5° threshold, 0.7× for lower corners)
  - Per-motor bottom limit switches
- **Ullage Monitoring**:
  - Continuously measures distance with HC-SR04
  - Sensor validation: 2-200cm range (invalid readings use last valid value)
  - **If ullage > 15cm**: Platform STOPS, returns to TOP (material far away, sufficient space)
  - **If ullage ≤ 15cm**: Platform CONTINUES lowering (material close, making space)
- **Transitions**:
  - ullage > 15cm → TOP (stops, waits for material to pile up)
  - All bottom limits triggered → BOTTOM
  - `cmd=stop` → TOP

#### **BOTTOM** (Fully Lowered)
- **Entry**: From LOWERING when all bottom limit switches triggered
- **Purpose**: Platform at lowest position, ready to raise
- **Motor Control**: All motors stopped
- **Transitions**:
  - `cmd=up` → RAISING
  - Manual motor commands allowed

#### **RAISING** (Moving Up)
- **Entry**: From BOTTOM state
- **Purpose**: Raise platform back to top position
- **Motor Control**:
  - All motors move up (clockwise) at same speed
  - No auto-leveling (not needed for upward movement)
  - Per-motor top limit switches
- **Transitions**:
  - All top limits triggered → TOP
  - `cmd=stop` → TOP

---

## Auto-Leveling Algorithm

### High-Level Flow

#### Auto-Start and Auto-Leveling with Ullage Tracking
```
1. System monitors ullage continuously in TOP state
2. If ullage < 15cm: Auto-start → Transition to LOWERING
   OR
   Receive manual command: "Platform: cmd=down,pwm=200"
3. Validate and clamp PWM (123-253 range)
4. Transition to LOWERING state
5. Loop every cycle:
   a. Read ullage (already measured in main loop)
   b. Validate sensor reading (2-200cm range)
      - If invalid: Use last valid ullage, log error
   c. Check ullage threshold:
      - If ullage > 15cm: STOP motors, transition to TOP (material far, sufficient space)
      - If ullage ≤ 15cm: CONTINUE lowering (material close, making space)
   d. Read gyro sensors (pitch & roll angles)
   e. Calculate speed compensation for each motor based on tilt
   f. Clamp individual motor speeds (60-253 range)
   g. Apply motor commands with adjusted speeds
   h. Check individual bottom limit switches
   i. Stop individual motors as they reach bottom
   j. Exit when all motors reach bottom (→ BOTTOM state)
```

#### Simple Up Movement
```
1. Receive command: "Platform: cmd=up,pwm=200"
2. Validate and clamp PWM (123-253 range)
3. Transition to RAISING state
4. Loop every cycle:
   a. All motors move up at base speed (no auto-leveling)
   b. Check individual top limit switches
   c. Stop individual motors as they reach top
   d. Exit when all motors reach top (→ TOP state)
```

### Detailed Algorithm

#### Step 1: Command Reception and PWM Validation
**Location**: [lowering-platform.h:101-134](include/lowering-platform.h#L101-L134)

```cpp
Command formats:
  "Platform: cmd=down,pwm=200"
  "Platform: cmd=up,pwm=150"
  "Platform: cmd=stop"

Parsing:
- Extract cmd parameter ("down", "up", or "stop")
- Extract pwm parameter (base speed, default 250)
- Validate PWM range: constrain(base_speed, 123, 253)
  - Maximum: 253 PWM
  - Minimum: 123 PWM (ensures 0.49× compensation ≥ 60 PWM)

Actions:
- If cmd == "down":
  - Set current_operation = LOWERING
  - Store platform_base_speed = validated pwm value
  - Clear command to prevent re-parsing
  - State machine calls platformAutoLevel() each loop

- If cmd == "up":
  - Set current_operation = RAISING
  - Store platform_base_speed = validated pwm value
  - Clear command to prevent re-parsing
  - State machine calls platformMoveUp() each loop

- If cmd == "stop":
  - Set current_operation = TOP (emergency stop)
  - Call stopAllMotors()
  - Exit all platform movement modes
```

#### Step 2: Ullage Validation and Monitoring (LOWERING State Only)
**Location**: [lowering-platform.h:293-374](include/lowering-platform.h#L293-L374)

```cpp
Read current ullage (already measured in main loop):
- current_ullage = Hcsr04::distance_cm

Validate sensor reading:
- If current_ullage < 2.0cm OR current_ullage > 200.0cm:
  - Invalid reading (sensor error or out of range)
  - Use last valid reading: current_ullage = last_ullage_cm
  - Log: "Sensor error - using last valid ullage"

Check ullage threshold:
- If current_ullage > ullage_threshold_cm (15cm):
  - Material is far from sensor (sufficient space already)
  - Set all motor.cmd = "stop"
  - Transition: current_operation = TOP
  - Log: "Ullage above threshold - STOPPED, sufficient space for material"
  - Return (exit function, motors stopped, will auto-restart when material piles up)

- If current_ullage ≤ ullage_threshold_cm (15cm):
  - Material is close to sensor (need to make space)
  - Log: "Material close, continuing descent to make space"
  - Continue to gyro reading and auto-leveling
  - Store: last_ullage_cm = current_ullage
```

#### Step 3: Gyro Reading (Auto-Leveling Down Only)
**Location**: [lowering-platform.h:308-310](include/lowering-platform.h#L308-L310)

```cpp
Read current tilt angles:
- pitch = Gyro::getPitch()  // Forward/Backward tilt
- roll = Gyro::getRoll()    // Left/Right tilt

Update frequency: 500ms (configured in timing.h)
```

#### Step 4: Speed Calculation (Auto-Leveling Down Only)
**Location**: [lowering-platform.h:316-357](include/lowering-platform.h#L316-L357)

```
Initialize all motor speeds to base speed:
motor1_speed = platform_base_speed
motor2_speed = platform_base_speed
motor3_speed = platform_base_speed
motor4_speed = platform_base_speed

PITCH COMPENSATION (Forward/Back tilt):
├─ If pitch > +5.0°:
│  ├─ Condition: Front is tilted down (lower than back)
│  ├─ Action: Slow down front motors to let back catch up
│  ├─ motor1_speed = platform_base_speed × 0.7
│  └─ motor2_speed = platform_base_speed × 0.7
│
└─ If pitch < -5.0°:
   ├─ Condition: Back is tilted down (lower than front)
   ├─ Action: Slow down back motors to let front catch up
   ├─ motor3_speed = platform_base_speed × 0.7
   └─ motor4_speed = platform_base_speed × 0.7

ROLL COMPENSATION (Left/Right tilt):
├─ If roll > +5.0°:
│  ├─ Condition: Right side is tilted down (lower than left)
│  ├─ Action: Slow down right motors to let left catch up
│  ├─ motor2_speed = motor2_speed × 0.7  (compound with pitch)
│  └─ motor4_speed = motor4_speed × 0.7  (compound with pitch)
│
└─ If roll < -5.0°:
   ├─ Condition: Left side is tilted down (lower than right)
   ├─ Action: Slow down left motors to let right catch up
   ├─ motor1_speed = motor1_speed × 0.7  (compound with pitch)
   └─ motor3_speed = motor3_speed × 0.7  (compound with pitch)

Note: Roll compensation is applied AFTER pitch compensation,
      so motors can be slowed down twice (0.7 × 0.7 = 0.49 = 49% speed)
      if both pitch and roll indicate that corner is lower.

PWM CLAMPING (Safety limits):
After all compensation calculations, clamp each motor speed:
├─ motor1_speed = constrain(motor1_speed, 60, 253)
├─ motor2_speed = constrain(motor2_speed, 60, 253)
├─ motor3_speed = constrain(motor3_speed, 60, 253)
└─ motor4_speed = constrain(motor4_speed, 60, 253)

This ensures:
- No motor runs below 60 PWM (minimum for reliable operation)
- No motor exceeds 253 PWM (maximum safe speed)
```

**Speed Compensation Examples (Base PWM = 200):**

| Scenario | Pitch | Roll | M1 Speed | M2 Speed | M3 Speed | M4 Speed |
|----------|-------|------|----------|----------|----------|----------|
| Level platform | 2° | 1° | 200 (100%) | 200 (100%) | 200 (100%) | 200 (100%) |
| Front tilted down | 8° | 0° | 140 (70%) | 140 (70%) | 200 (100%) | 200 (100%) |
| Right tilted down | 0° | 7° | 200 (100%) | 140 (70%) | 200 (100%) | 140 (70%) |
| Front-right corner low | 6° | 6° | 140 (70%) | 98 (49%) | 200 (100%) | 140 (70%) |
| Back-left corner low | -7° | -8° | 98 (49%) | 200 (100%) | 140 (70%) | 200 (100%) |

#### Step 5: Motor Command Assignment
**Location**: [lowering-platform.h:297-305](include/lowering-platform.h#L297-L305)

```cpp
Set all motors to descend (anticlockwise) with calculated speeds:

motor1.cmd = "anticlockwise"
motor1.speed = motor1_speed

motor2.cmd = "anticlockwise"
motor2.speed = motor2_speed

motor3.cmd = "anticlockwise"
motor3.speed = motor3_speed

motor4.cmd = "anticlockwise"
motor4.speed = motor4_speed
```

#### Step 6: Per-Motor Limit Switch Checking
**Location**: [lowering-platform.h:307-327](include/lowering-platform.h#L307-L327)

```cpp
Check each bottom limit switch (active LOW with INPUT_PULLUP):

For each motor (1-4):
  If bottom_limit_sw_N_val == 0 (switch pressed):
    - Set motorN.cmd = "stop"
    - Log "Motor N bottom limit reached"
    - Motor N stops independently
    - Other motors continue descending

This allows the platform to continue leveling even after
some corners reach bottom, preventing binding or damage.
```

#### Step 7: Completion Detection
**Location**: [lowering-platform.h:329-337](include/lowering-platform.h#L329-L337)

```cpp
Check if ALL motors have reached bottom:

If (bottom_limit_sw_1_val == 0 AND
    bottom_limit_sw_2_val == 0 AND
    bottom_limit_sw_3_val == 0 AND
    bottom_limit_sw_4_val == 0):

  Actions:
  - Set platformAutoLeveling = false
  - Call stopAllMotors()
  - Log "All motors reached bottom - auto-leveling complete"
  - Exit auto-leveling mode
  - Return from function
```

#### Step 8: Motor Execution
**Location**: [lowering-platform.h:339-343](include/lowering-platform.h#L339-L343)

```cpp
Execute motor control functions with updated commands and speeds:

motor1Ctrl()  // Applies motor1.cmd and motor1.speed
motor2Ctrl()  // Applies motor2.cmd and motor2.speed
motor3Ctrl()  // Applies motor3.cmd and motor3.speed
motor4Ctrl()  // Applies motor4.cmd and motor4.speed

Each function sets appropriate GPIO pins and PWM values
based on the command (clockwise/anticlockwise/stop) and speed.
```

#### Step 9: Diagnostic Logging
**Location**: [lowering-platform.h:345-360](include/lowering-platform.h#L345-L360)

```cpp
Every 1 second (when updateSwitchesLogging timer triggers):

Log the following to Serial:
- Current pitch angle (degrees)
- Current roll angle (degrees)
- Calculated speed for Motor 1
- Calculated speed for Motor 2
- Calculated speed for Motor 3
- Calculated speed for Motor 4

Example output:
"Tilt - Pitch: 7.2° Roll: 3.1° Speeds: M1=84 M2=84 M3=120 M4=120"
```

---

## Safety Features

### 1. Per-Motor Limit Switch Control
During auto-leveling mode, each motor responds only to its own limit switch:
- **Individual stop**: Motor stops when its bottom limit is reached
- **Others continue**: Other motors keep running to level the platform
- **Prevents binding**: Allows platform to fully settle without mechanical stress

### 2. Global Limit Switch Safety (Non-Auto-Leveling)
**Location**: [lowering-platform.h:400-415](include/lowering-platform.h#L400-L415)

When NOT in auto-leveling mode:
- **Top limit switches**: ANY top switch triggers stops ALL motors (safety)
- **Bottom limit switches**: ANY bottom switch triggers stops ALL motors (safety)
- **Purpose**: Protects against runaway during manual motor control

### 3. Emergency Stop
At any time, send command: `Platform: cmd=stop`
- Immediately stops all motors
- Exits auto-leveling mode
- Can be used as emergency stop

### 4. Gyro Sensor Failure Handling
If gyro fails to update:
- Motors continue at base speed (no compensation)
- System degrades gracefully to equal-speed descent
- Manual stop command still functional

---

## Configuration Parameters

### Adjustable Parameters

| Parameter | Location | Default | Description |
|-----------|----------|---------|-------------|
| **PWM Limits** | | | |
| Maximum PWM | [lowering-platform.h:100](include/lowering-platform.h#L100) | 253 | Maximum allowed PWM speed |
| Minimum Base PWM | [lowering-platform.h:100](include/lowering-platform.h#L100) | 123 | Minimum base PWM (ensures 60 after 0.49× compensation) |
| Minimum Motor PWM | [lowering-platform.h:319-322](include/lowering-platform.h#L319-L322) | 60 | Minimum PWM after compensation applied |
| **Auto-Leveling** | | | |
| Tilt Threshold | [lowering-platform.h:281,288,300,307](include/lowering-platform.h#L281) | 5.0° | Minimum tilt angle to trigger compensation |
| Speed Reduction Factor | [lowering-platform.h:284,291,303,310](include/lowering-platform.h#L284) | 0.7 (70%) | Speed multiplier for lower corners |
| **Ullage Monitoring** | | | |
| Ullage Threshold | [lowering-platform.h:12](include/lowering-platform.h#L12) | 15.0 cm | Clearance threshold - platform operates when ullage < threshold |
| Last Valid Ullage | [lowering-platform.h:11](include/lowering-platform.h#L11) | 0.0 cm | Last valid ullage reading for filtering |
| Sensor Min Valid | [lowering-platform.h:13](include/lowering-platform.h#L13) | 2.0 cm | Minimum valid sensor reading |
| Sensor Max Valid | [lowering-platform.h:14](include/lowering-platform.h#L14) | 200.0 cm | Maximum valid sensor reading |
| **Timing** | | | |
| Base Speed Default | [lowering-platform.h:38](include/lowering-platform.h#L38) | 254 | Default PWM if not specified in command |
| Gyro Update Rate | [timing.h:69](include/timing.h#L69) | 500ms | How often gyro is read |
| Log Update Rate | [actuators-and-sensors.h:71](include/actuators-and-sensors.h#L71) | 1000ms | Diagnostic logging interval |

### PWM Range Table

| Input PWM | After Validation | Best Case (100%) | With 0.7× | With 0.49× (worst) | Notes |
|-----------|-----------------|------------------|-----------|-------------------|-------|
| 50 | 123 (clamped up) | 123 | 86 | 60 (clamped) | Too low, clamped to minimum |
| 123 | 123 | 123 | 86 | 60 (clamped) | Minimum valid base |
| 150 | 150 | 150 | 105 | 73 | Recommended minimum |
| 200 | 200 | 200 | 140 | 98 | Recommended default |
| 253 | 253 | 253 | 177 | 124 | Maximum allowed |
| 255 | 253 (clamped down) | 253 | 177 | 124 | Too high, clamped to maximum |

### Tuning Recommendations

**If platform oscillates (overshoots level):**
- Increase speed reduction factor (try 0.5 or 0.6)
- Decrease tilt threshold (try 3° or 4°)
- Increase gyro update rate (try 250ms or 300ms)

**If platform levels too slowly:**
- Decrease speed reduction factor (try 0.8 or 0.85)
- Increase tilt threshold (try 7° or 10°)
- Increase base speed in command

**If platform drifts after leveling:**
- Decrease tilt threshold (more sensitive)
- Add deadband around 0° (±1°) where no compensation occurs
- Calibrate gyro sensor offset

**If platform starts/stops too frequently (ullage monitoring):**
- Increase ullage_threshold_cm from 15cm to 20-25cm
- Provides wider operating margin
- Useful for uneven material surfaces or noisy sensor readings

**If platform doesn't start when material is close:**
- Decrease ullage_threshold_cm from 15cm to 10-12cm
- Allows closer operation before triggering
- Check sensor is measuring correctly

**If sensor gives erratic readings:**
- Adjust valid_ullage_min and valid_ullage_max (currently 2-200cm)
- Check sensor mounting - must be perpendicular to material surface
- Ensure material surface is relatively flat
- Add shielding from electrical noise
- Verify 5V power supply is stable

**If platform pulses rapidly (start/stop/start):**
- Sensor filtering already implemented (2-200cm range)
- Check for loose sensor connections
- Increase ullage_threshold_cm for more tolerance
- Verify material surface is not highly reflective or absorbent

---

## Usage Examples

### Example 1: Automatic Continuous Material Filling
```
Scenario: Platform auto-starts and tracks material level

Initial State:
- Ullage = 50cm → Platform at TOP (idle)

Material starts piling up:
Serial output:
"Auto-start: Ullage (12.0 cm) < threshold (15.0 cm), material close - starting platform lowering"
"Ullage: 12.0 cm - material close, continuing descent to make space"
"Tilt - Pitch: 2.1° Roll: 1.3° Speeds: M1=120 M2=120 M3=120 M4=120"
"Ullage: 10.5 cm - material close, continuing descent to make space"
"Pitch: Front lower, slowing motors 1&2"
"Tilt - Pitch: 6.5° Roll: 0.8° Speeds: M1=84 M2=84 M3=120 M4=120"

Material feed slows/stops (ullage increases):
"Ullage above threshold (18.5 cm > 15.0 cm) - STOPPED, sufficient space for material"
[Platform at TOP, motors stopped]

Material feed resumes (auto-restart):
"Auto-start: Ullage (13.2 cm) < threshold (15.0 cm), material close - starting platform lowering"
"Ullage: 12.8 cm - material close, continuing descent to make space"
[Cycle repeats]

Eventually reaches bottom:
"Motor 1 bottom limit reached"
"Motor 2 bottom limit reached"
"Motor 3 bottom limit reached"
"Motor 4 bottom limit reached"
"All motors reached bottom - platform at BOTTOM"
```

### Example 2: Simple Descent Without Ullage Monitoring
```
For applications not requiring ullage monitoring, the same commands work:

Command: Platform: cmd=down,pwm=200

Expected behavior:
- All motors descend with auto-leveling
- Ullage checks still occur but may never trigger if material fills quickly
- Platform reaches BOTTOM when all limit switches active
```

### Example 3: Emergency Stop During Operation
```
1. Command: Platform: cmd=down,pwm=120
   [Platform starts descending with auto-leveling, state: LOWERING]

2. Command: Platform: cmd=stop
   [All motors immediately stop]
   [State transitions to: TOP]
   Serial output: "Platform: Emergency stop - returning to TOP"
```

### Example 4: Platform Move Up
```
Command: Platform: cmd=up,pwm=180

Expected behavior:
1. All motors move up at 180 PWM (state: RAISING)
2. No auto-leveling (all motors same speed)
3. Each motor stops individually when its top limit switch triggers
4. Continues until all 4 motors reach top
5. Transitions to TOP state

Serial output:
"Platform: Raising"
"Motor 1 top limit reached"
"Motor 3 top limit reached"
"Motor 2 top limit reached"
"Motor 4 top limit reached"
"All motors reached top - platform at TOP"
```

### Example 5: Manual Motor Control (No Auto-Leveling)
```
Command: Motor1: cmd=anticlockwise,pwm=150

Expected behavior:
- Only Motor 1 descends at 150 PWM
- No auto-leveling active
- Global limit switch safety applies (ANY limit stops ALL)
```

---

## Auto-Start and Ullage Tracking Algorithm

### Function: checkUllageAndAutoStart()
**Location**: [lowering-platform.h:568-591](include/lowering-platform.h#L568-591)

```
Purpose: Monitor ullage in TOP state and automatically start lowering when material piles up
close to sensor

Algorithm (executed every loop cycle when state = TOP):
1. Read current ullage (already measured in main loop):
   - current_ullage = Hcsr04::distance_cm

2. Validate sensor reading:
   - If current_ullage < 2.0cm OR current_ullage > 200.0cm:
     - Invalid reading (sensor error)
     - Skip this cycle, return without action
     - Prevents false triggers from erratic readings

3. Check if material is piling up close to sensor:
   - If current_ullage < ullage_threshold_cm (15cm):
     - Material is close to sensor, needs space
     - Transition: current_operation = LOWERING
     - Set: platform_base_speed = 200 (default auto-start speed)
     - Log: "Auto-start: Ullage (X cm) < threshold (15 cm), material close - starting platform lowering"
     - Next loop cycle will enter platformAutoLevel() automatically
```

**Operation Example:**
```
Scenario: Continuous Material Filling from Below (e.g., conveyor belt)

System Start:
- Ullage = 50cm → Platform at TOP (idle, sufficient space)

Material Starts Piling Up:
- Ullage = 30cm → Still at TOP (sufficient space)
- Ullage = 20cm → Still at TOP (sufficient space)
- Ullage = 14cm → AUTO-START! → Transition to LOWERING

Platform Lowering:
- Ullage = 12cm → Continue lowering (making space)
- Ullage = 10cm → Continue lowering (making space)
- Ullage = 18cm → STOP! → Transition to TOP (material slowed/stopped, sufficient space)

Material Piles Up Again:
- Ullage = 14cm → AUTO-START! → Resume lowering
- Cycle repeats until BOTTOM reached
```

**Sensor Filtering Example:**
```
Invalid Readings Handled:
- Ullage = 0.00cm → IGNORED (below minimum 2cm, likely sensor error)
- Ullage = 239.57cm → IGNORED (above maximum 200cm, likely sensor error)
- Ullage = 12.5cm → VALID → Auto-start triggered

Prevents rapid start/stop pulsing from erratic sensor readings
```

**Key Features:**
- Fully automatic operation - no manual start command needed
- Sensor validation prevents false triggers from erratic readings
- Platform "chases" material surface, maintaining clearance
- Adapts to variable material fill rates
- Stops when material is far (saves energy)
- Resumes when material piles up close

**Practical Application:**
This is ideal for continuous material filling operations where:
- Material is constantly being added from below (conveyor, hopper, chute)
- Platform must make space as material accumulates
- System operates autonomously without human intervention
- Examples: Bin filling, silo loading, trash compaction, material handling

---

## Platform Move Up Algorithm

### Function: platformMoveUp()
**Location**: [lowering-platform.h:381-430](include/lowering-platform.h#L381-L430)

```
Purpose: Move all motors upward with per-motor limit switch control
No auto-leveling applied during upward movement

Algorithm:
1. Set all motors to clockwise (up) direction
2. Set all motor speeds to platform_base_speed
3. Check each top limit switch:
   - If top_limit_sw_N_val == 0 (pressed):
     - Set motorN.cmd = "stop"
     - Log "Motor N top limit reached"
4. Check if all motors reached top:
   - If all top switches pressed:
     - Set platform_moving_up = false
     - Call stopAllMotors()
     - Log "All motors reached top - move up complete"
     - Exit function
5. Execute motor control functions for all 4 motors
```

**Key Features:**
- Simple equal-speed upward movement
- Individual motor stop at top limit
- No gyro compensation (not needed for upward movement)
- Uses same PWM validation as down movement (123-253 range)

---

## Algorithm Advantages & Trade-offs

### Advantages
1. **Simple & Robust**: Speed compensation is easy to understand and debug
2. **No Complex Math**: Avoids PID tuning, control theory, or complex calculations
3. **Predictable**: Deterministic behavior makes testing straightforward
4. **Graceful Degradation**: If gyro fails, motors continue at equal speed
5. **Hardware Tolerant**: Works despite motor variations and mechanical imperfections

### Trade-offs
1. **Descent Time**: Slowing down lower corners increases total descent time
   - Alternative: Speed up higher corners (but may exceed safe speed limits)
2. **Step Response**: 70% reduction is a fixed step, not gradual
   - Can cause slight oscillation if tilt is near threshold
   - Alternative: Proportional compensation based on tilt angle
3. **Threshold Sensitivity**: 5° threshold is somewhat arbitrary
   - Requires field testing to optimize for specific platform
4. **No Feed-forward**: Purely reactive, doesn't predict tilt changes
   - Alternative: Add acceleration-based prediction

---

## Future Enhancements

### Potential Improvements
1. **Proportional Speed Control**
   ```cpp
   // Instead of fixed 0.7 multiplier:
   float compensation = map(abs(tilt), 5.0, 15.0, 1.0, 0.5);
   motor_speed = platformBaseSpeed * compensation;
   ```

2. **Upward Auto-Leveling**
   - Implement `Platform: cmd=up,pwm=120`
   - Use same algorithm but with clockwise direction
   - Check top limit switches instead of bottom

3. **Tilt Rate Limiting**
   - Detect if tilt is changing too fast
   - Reduce speed globally if unstable
   - Prevent oscillation or runaway tilt

4. **Load Compensation**
   - Detect if platform is loaded unevenly
   - Adjust speed factors based on expected vs actual tilt rate
   - Learn motor characteristics over time

5. **Serial Status Reporting**
   - Send tilt and speed data back to Serial1
   - Allow remote monitoring via host computer
   - JSON format for easy parsing

6. **Configuration via Serial**
   ```cpp
   Config: threshold=5.0,factor=0.7,rate=500
   ```
   - Allow runtime parameter adjustment
   - Save to EEPROM for persistence
   - No need to recompile firmware

---

## Troubleshooting

### Problem: Platform doesn't level, all motors same speed
**Possible Causes:**
- Gyro not updating (check [main.cpp:37-41](src/main.cpp#L37-L41))
- Tilt below 5° threshold
- Gyro orientation wrong (swap pitch/roll logic)

**Solutions:**
- Verify gyro I2C connection (address 0x68)
- Lower tilt threshold to 3°
- Add debug logging for pitch/roll values

### Problem: Platform oscillates back and forth
**Possible Causes:**
- Speed reduction too aggressive
- Gyro update rate too slow
- Threshold too sensitive

**Solutions:**
- Change speed factor from 0.7 to 0.8
- Increase gyro update rate from 500ms to 250ms
- Add deadband: don't compensate if tilt < 2°

### Problem: One motor doesn't stop at limit
**Possible Causes:**
- Limit switch wiring issue
- Switch mechanically failed
- Pin number incorrect in code

**Solutions:**
- Test switch: check `bottom_limit_sw_N_val` in serial log
- Verify INPUT_PULLUP mode (switch should read 1 normally, 0 when pressed)
- Check pin definitions in [actuators-and-sensors.h:31-34](include/actuators-and-sensors.h#L31-L34)

### Problem: Motors run at different speeds when level
**Possible Causes:**
- Motor characteristics vary (different no-load speeds)
- Mechanical friction differences
- Power supply voltage drop under load

**Solutions:**
- Calibrate per-motor speed offsets
- Use closed-loop control (encoders) instead of open-loop PWM
- Add load compensation algorithm

### Problem: Motors won't move or move too slowly
**Possible Causes:**
- PWM too low (below motor starting threshold ~50-70 PWM)
- Input PWM clamped to minimum (123)
- Compensation reducing speed below 60 PWM

**Solutions:**
- Use higher base PWM (150-200 recommended)
- Check if input PWM < 123 (will be clamped up automatically)
- Verify motors can run at 60 PWM minimum
- Increase minimum PWM clamp from 60 to 80 if needed

### Problem: Motors running too fast / uncontrollable
**Possible Causes:**
- Input PWM > 253 (will be clamped down automatically)
- Base speed too high for mechanical system

**Solutions:**
- Reduce input PWM in command
- Maximum is 253 PWM (enforced by code)
- Consider adding mechanical brake or slower gearing

### Problem: Platform doesn't stop when ullage threshold reached
**Possible Causes:**
- Ultrasonic sensor not measuring correctly
- Sensor wiring issue (trigger pin 45, echo pin 44)
- Material surface too irregular (sensor gets inconsistent readings)
- ullage_threshold_cm set to 0 or very low value

**Solutions:**
- Test sensor: check Hcsr04::distance_cm values in serial monitor
- Verify sensor is pointing downward at material surface
- Ensure sensor has clear line of sight (no obstructions)
- Check wiring connections for trigger and echo pins
- Try increasing ullage_threshold_cm for testing

### Problem: Platform doesn't resume after material fills
**Possible Causes:**
- Material not filling high enough (distance_change < 20cm)
- Ultrasonic sensor reading incorrectly
- Platform stuck in WAITING_FILL state

**Solutions:**
- Add debug logging in checkUllageAndContinue() to see distance values
- Verify: current_distance - last_distance_stopped_cm ≥ 20cm
- Check that material surface is relatively flat and reflective
- Temporarily reduce ullage_threshold_cm for testing
- Send `cmd=stop` then `cmd=down` to reset if stuck

### Problem: Ultrasonic sensor shows erratic readings
**Possible Causes:**
- Sensor too close to material (< 2cm minimum distance)
- Material surface not reflective or too angled
- Electrical noise interference
- Sensor measurement frequency too high

**Solutions:**
- Ensure minimum 2cm clearance from sensor to material
- Material should be relatively flat and perpendicular to sensor
- Add filtering: average multiple readings before checking threshold
- Check power supply quality and ground connections

---

## Testing Checklist

### Initial Testing (Safety First)
- [ ] Verify all limit switches work correctly
- [ ] Test emergency stop command
- [ ] Confirm motors stop when limit reached
- [ ] Check gyro readings are reasonable (±10° when tilted by hand)
- [ ] Test ultrasonic sensor: verify distance readings in serial monitor
- [ ] Confirm sensor detects material surface correctly

### Functional Testing
- [ ] Level platform descends straight down
- [ ] Tilted platform (front down) compensates correctly
- [ ] Tilted platform (right down) compensates correctly
- [ ] Diagonal tilt (front-right down) compensates correctly
- [ ] All motors reach bottom and stop
- [ ] Auto-leveling exits properly when complete

### Ullage Monitoring Testing
- [ ] Platform stops when ullage drops below 20cm threshold
- [ ] State transitions from LOWERING to WAITING_FILL
- [ ] Platform remains stopped while waiting for material
- [ ] Platform automatically resumes when ullage increases by 20cm
- [ ] State transitions from WAITING_FILL back to LOWERING
- [ ] Cycle repeats correctly for multiple stop/resume sequences
- [ ] Platform reaches BOTTOM state after all cycles

### Edge Cases
- [ ] Platform starts at extreme tilt (>20°)
- [ ] One motor reaches bottom early (others continue)
- [ ] Stop command during active descent
- [ ] Stop command while in WAITING_FILL state
- [ ] Rapid tilt changes (manual push while descending)
- [ ] Gyro disconnected (fail-safe behavior)
- [ ] Ultrasonic sensor blocked or disconnected

### Performance Tuning
- [ ] Measure descent time with auto-leveling vs without
- [ ] Verify max tilt angle during descent stays < 10°
- [ ] Check for oscillation or hunting behavior
- [ ] Optimize speed factor for fastest level descent
- [ ] Test different ullage thresholds (10cm, 15cm, 20cm, 25cm)
- [ ] Verify automatic stop/resume cycle timing matches fill rate

---

## Conclusion

This enhanced auto-leveling system provides a comprehensive solution for maintaining platform level during descent with integrated ullage monitoring for automatic material filling applications. The system combines gyroscope-based tilt compensation with ultrasonic distance sensing to create a fully automated, adaptive platform control system.

### Key Design Decisions:

1. **Speed reduction** (not speed increase) for lower corners
   - Lower corners slow to 70% of base speed
   - Higher corners maintain 100% base speed
   - Prevents motors from exceeding safe maximum speed

2. **Fixed 70% factor** (not proportional) for simplicity
   - Easy to understand and debug
   - Predictable behavior across all tilt angles
   - Can be compound (0.49×) for diagonal tilts

3. **Per-motor limit switches** during platform operations
   - Each motor stops independently when limit reached
   - Allows platform to fully settle without binding
   - Only during Platform commands (auto-leveling and move-up)
   - Global stop still applies for manual motor control

4. **5° tilt threshold** to avoid noise
   - Sensitive enough for effective leveling
   - Not so sensitive that sensor noise triggers false compensation
   - Field-tested optimal value

5. **Strict PWM limits** (60-253 range)
   - Input validation: 123-253 PWM (ensures minimum 60 after worst-case 0.49× compensation)
   - Output clamping: 60-253 PWM per motor (ensures reliable motor operation)
   - Prevents stalled motors (< 60 PWM) and unsafe speeds (> 253 PWM)

6. **Enum-based state machine** (not booleans)
   - Clear state transitions: TOP → LOWERING → WAITING_FILL → LOWERING → ... → BOTTOM → RAISING → TOP
   - Single source of truth for system state
   - Easy to debug and extend

7. **Ullage monitoring with automatic stop/resume**
   - 20cm threshold prevents platform from contacting material prematurely
   - Automatic resume when material fills by threshold amount
   - Fully autonomous operation for continuous filling applications
   - Adaptable to varying fill rates

8. **Per-motor limit control during operations**
   - Each motor stops independently at its limit
   - Global safety stops only during manual control (TOP, WAITING_FILL, BOTTOM states)
   - Prevents binding and allows proper settling

### Operation Summary:

**For Automatic Material Filling:**
- Command: `Platform: cmd=down,pwm=200`
- Result: Platform lowers with auto-leveling, stops when ullage < 20cm, automatically resumes when material fills
- Use Case: Continuous bin filling, hopper loading, automated compaction

**For Simple Upward Movement:**
- Command: `Platform: cmd=up,pwm=200`
- Result: Fast equal-speed ascent with per-motor stops
- No leveling compensation needed when lifting

**For Emergency Situations:**
- Command: `Platform: cmd=stop`
- Result: Immediate stop, return to TOP state
- Works in any state (LOWERING, WAITING_FILL, RAISING)

### Advantages Over Previous Implementation:

✅ **Fully Automatic Operation**: No manual intervention needed during fill cycles
✅ **State Machine Clarity**: Enum-based design eliminates boolean flag confusion
✅ **Ullage-Based Control**: Maintains safe clearance from material surface
✅ **Adaptive Behavior**: Responds to varying fill rates automatically
✅ **Enhanced Safety**: Prevents collisions with material during filling
✅ **Predictable Cycles**: Clear state transitions make debugging easier

### Typical Applications:

- **Automated Bin Filling**: Platform lowers as material is added continuously
- **Silo Loading**: Maintains level descent while hopper fills from conveyor
- **Material Compaction**: Platform descends as material settles and compresses
- **Batch Processing**: Stop/start cycles align with batch delivery timing
- **Quality Control**: Prevents overflow by maintaining minimum ullage threshold

This implementation provides industrial-grade reliability for automated material handling operations where both precision leveling and intelligent fill control are critical requirements.
