# Bucket Tipping System Documentation

## Overview
This document describes the automatic bucket tipping system for a bucket suspended by 4 pulleys, each controlled by an independent motor at each corner. The system automatically tips the bucket **backward** to approximately 80 degrees when it reaches the bottom position (above a bin), holds for 3 seconds to empty contents into the bin below, then raises it back up with auto-leveling control. The bucket lowers to varying heights as the bin fills up, with the bottom limit switches triggering the tipping sequence at the current position.

---

## System Architecture

### Hardware Components
- **4 DC Motors**: Each controls one corner pulley (front-left, front-right, back-left, back-right)
- **8 Limit Switches**: Top and bottom switches for each of the 4 motors
- **MPU6050 Gyroscope**: Measures bucket tilt angle (pitch and roll)
- **Arduino Mega 2560**: Main controller
- **Serial Communication**: Command input via Serial1 (9600 baud)

### Motor-Corner Mapping
```
        FRONT (Raises Up)
    [M1]      [M2]
      ↑        ↑

LEFT              RIGHT

      ↓        ↓
    [M3]      [M4]
        BACK (Stays at Bottom - Pivot Point)
```

- **Motor 1**: Front-Left (raises up during tipping)
- **Motor 2**: Front-Right (raises up during tipping)
- **Motor 3**: Back-Left (holds at bottom during tipping - acts as pivot)
- **Motor 4**: Back-Right (holds at bottom during tipping - acts as pivot)

**Tipping Motion**: Front edge (M1, M2) rises while back edge (M3, M4) stays at bottom, causing bucket to tip backward and pour contents down into bin below.

### Gyroscope Orientation
- **Pitch**: Forward (+) / Backward (-) tilt
  - **Negative pitch** → Back tilted down (bucket tipping backward to pour)
  - Target: **-75 to -80°** for full tipping
- **Roll**: Right (+) / Left (-) tilt
  - Used during raising phase for auto-leveling

---

## Implementation Files

### Modified File
**[include/bucket.h](include/bucket.h)**
- Lines 6-7: Added `#include "gyro.h"` and `#include "timing.h"`
- Lines 22-23: Added function declarations (`bucketTip()`, `bucketRaise()`)
- Lines 28-39: Added state machine variables and constants
- Lines 102-173: Added Bucket serial command handler (down, up, tip, stop)
- Lines 175-198: Added state machine logic in `forever()` loop
- Lines 290-328: Implemented `bucketTip()` function (backward tipping)
- Lines 330-419: Implemented `bucketRaise()` function with auto-leveling
- Lines 422-460: Updated `monitorSwitches()` for state-based control

---

## Serial Commands

### Command Format
All bucket commands follow this format:
```
Bucket: cmd=<command>,pwm=<speed>
```

### Available Commands

#### 1. Lower Bucket (Manual Down)
```
Bucket: cmd=down,pwm=200
```
- **Function**: Manually lower bucket at specified speed
- **Motors**: All 4 motors move down (anticlockwise) at same speed
- **PWM Range**: 123-253 (automatically clamped)
- **Default PWM**: 200 if not specified
- **State**: Requires IDLE state
- **Use Case**: Position bucket above bin before automatic tipping

**Example**:
```
Bucket: cmd=down,pwm=180    // Lower at 180 PWM
Bucket: cmd=down            // Lower at default 200 PWM
```

#### 2. Raise Bucket (Manual Up)
```
Bucket: cmd=up,pwm=200
```
- **Function**: Manually raise bucket at specified speed
- **Motors**: All 4 motors move up (clockwise) at same speed
- **PWM Range**: 123-253 (automatically clamped)
- **Default PWM**: 200 if not specified
- **State**: Can interrupt any state (sets state to IDLE)
- **Use Case**: Return bucket to top position manually

**Example**:
```
Bucket: cmd=up,pwm=220      // Raise at 220 PWM
Bucket: cmd=up              // Raise at default 200 PWM
```

#### 3. Manual Tip Trigger
```
Bucket: cmd=tip
```
- **Function**: Manually trigger tipping sequence without limit switch
- **State Requirement**: Must be in IDLE state
- **Sequence**: IDLE → TIPPING → HOLDING → RAISING → IDLE
- **Safety**: Will not tip if already in TIPPING, HOLDING, or RAISING state
- **Use Case**: Test tipping sequence or manually trigger when needed

**Example**:
```
Bucket: cmd=tip             // Start tipping sequence immediately

Response:
- If IDLE: "Bucket: Manual tip sequence triggered"
- If busy: "Bucket: Cannot tip - currently in state <N>"
```

#### 4. Emergency Stop
```
Bucket: cmd=stop
```
- **Function**: Stop all motors immediately and reset to IDLE state
- **Motors**: All 4 motors stop
- **State**: Resets bucket_state to IDLE
- **Flags**: Clears all motor_running flags
- **Use Case**: Emergency stop or cancel ongoing operations

**Example**:
```
Bucket: cmd=stop            // Stop everything immediately

Response: "Bucket: Stopping all operations"
```

### Command Summary Table

| Command | Syntax | PWM | State Required | Action | Motors |
|---------|--------|-----|----------------|--------|--------|
| **down** | `Bucket: cmd=down,pwm=200` | 123-253 | IDLE | Manual lower | All down (anticlockwise) |
| **up** | `Bucket: cmd=up,pwm=200` | 123-253 | Any | Manual raise | All up (clockwise) |
| **tip** | `Bucket: cmd=tip` | N/A | IDLE | Start tipping sequence | State machine controlled |
| **stop** | `Bucket: cmd=stop` | N/A | Any | Emergency stop | All stop |

---

## Bucket Tipping Algorithm

### State Machine Overview

```
┌──────┐  Bottom Limit    ┌─────────┐  Pitch ≥ 75°   ┌─────────┐  3 sec elapsed   ┌─────────┐
│ IDLE │─────Triggered────>│ TIPPING │───────────────>│ HOLDING │──────────────────>│ RAISING │
└──────┘                   └─────────┘                └─────────┘                   └─────────┘
    ^                                                                                      │
    │                                                                                      │
    └──────────────────────────────────All Top Limits Reached─────────────────────────────┘
```

### State Descriptions

#### IDLE State
- **Purpose**: Normal operation, monitors for trigger condition
- **Motor Control**: Manual control via serial commands (Motor1-4: cmd=...)
- **Trigger**: ANY bottom limit switch goes LOW (pressed)
- **Action**: Transition to TIPPING state
- **Safety**: Global top limit switch stops all motors

#### TIPPING State
- **Purpose**: Tip bucket forward to approximately 80 degrees
- **Motor Control**:
  - Motors 1&2: Move down (anticlockwise) at TIPPING_SPEED (200 PWM)
  - Motors 3&4: Stop and hold position (act as pivot point)
- **Exit Condition**: Gyro pitch angle ≥ 75°
- **Action**: Stop motors 1&2, transition to HOLDING state
- **Duration**: Variable (depends on load, motor speed, mechanical resistance)

#### HOLDING State
- **Purpose**: Hold tipped position for 3 seconds to empty contents
- **Motor Control**: All motors stopped
- **Exit Condition**: 3000 milliseconds elapsed
- **Action**: Transition to RAISING state
- **Duration**: Fixed 3 seconds

#### RAISING State
- **Purpose**: Raise bucket back to level position with auto-leveling
- **Motor Control**:
  - All 4 motors move up (clockwise) with speed compensation
  - Auto-leveling adjusts individual motor speeds based on tilt
  - Per-motor top limit switch handling
- **Exit Condition**: All 4 top limit switches triggered
- **Action**: Stop all motors, transition to IDLE state
- **Duration**: Variable (depends on tilt, leveling adjustments)

---

## Detailed Algorithm

### Trigger Detection
**Location**: [bucket.h:431-440](include/bucket.h#L431-L440)

```cpp
Function: monitorSwitches() in IDLE state

Process:
1. Read all 8 limit switches
2. Check if ANY bottom limit switch is LOW (pressed):
   - bottom_limit_sw_1_val == 0 OR
   - bottom_limit_sw_2_val == 0 OR
   - bottom_limit_sw_3_val == 0 OR
   - bottom_limit_sw_4_val == 0
3. If trigger detected:
   - Log: "Bottom limit triggered - starting bucket tip sequence"
   - Set bucket_state = TIPPING
   - Exit function (skip global safety checks)
```

**Important**: The trigger is ANY bottom switch, not all. This allows the sequence to start even if the bucket is slightly tilted when it reaches bottom.

---

### TIPPING Phase

#### Function: bucketTip()
**Location**: [bucket.h:283-319](include/bucket.h#L283-L319)

```
Algorithm:

1. MOTOR CONTROL SETUP
   ├─ Motor 1: cmd = "clockwise", speed = TIPPING_SPEED (200)
   ├─ Motor 2: cmd = "clockwise", speed = TIPPING_SPEED (200)
   ├─ Motor 3: cmd = "stop"
   └─ Motor 4: cmd = "stop"

2. EXECUTE MOTOR COMMANDS
   ├─ Call motor1Ctrl() → Front-left raises up
   ├─ Call motor2Ctrl() → Front-right raises up
   ├─ Call motor3Ctrl() → Back-left holds at bottom
   └─ Call motor4Ctrl() → Back-right holds at bottom

3. READ GYRO SENSOR
   └─ pitch = Gyro::getPitch()  // Current forward/backward tilt angle

4. CHECK TILT ANGLE
   If pitch ≤ -75.0°:  // Negative pitch means back is down, bucket tipped backward
      ├─ Stop tipping motors:
      │  ├─ motor1.cmd = "stop"
      │  ├─ motor2.cmd = "stop"
      │  ├─ motor1Ctrl()
      │  └─ motor2Ctrl()
      ├─ Transition to HOLDING:
      │  ├─ bucket_state = HOLDING
      │  └─ hold_start_time = millis()
      └─ Log: "Bucket tipped backward to [angle] degrees, holding for 3 seconds"

5. REPEAT
   └─ Function is called every loop cycle until pitch ≤ -75°
```

**Key Points**:
- Target angle: **-75°** (negative = back tilted down, accounts for sensor accuracy)
- Front motors (1&2) **raise** the front edge
- Back motors (3&4) **hold at bottom** to act as pivot point
- Bucket tips **backward** to pour contents into bin below
- Gyro continuously monitors tilt angle
- Stops immediately when target angle reached
- Works at **varying heights** as bin fills up (bottom limit triggers at current position)

**Tipping Mechanics**:
```
Initial Position                Tipping in Progress              Final Position (-75 to -80°)
(Bucket at bottom, level):      (Front rising):                  (Tipped backward, pouring):

     [M1]─────[M2]                   ↑[M1]─────[M2]↑                    [M1]────[M2]
      |       |                       |         |                        ╱          ╱
      |       |                       |         |                      ╱          ╱
   ───┴───────┴───  BIN             ───┴─────────┴───  BIN          ╱  POURING ╱
      |       |     ▓▓▓               |         |     ▓▓▓       [M3]────[M4]  ←PIVOT
     [M3]    [M4]   ▓▓▓              [M3]══════[M4]   ▓▓▓           ▓▓▓▓▓▓  BIN
                    ▓▓▓               ↑HOLD AT BOTTOM  ▓▓▓           ▓▓▓▓▓▓ (Filling)
                    ▓▓▓              PIVOT POINT       ▓▓▓           ▓▓▓▓▓▓
```

**Operating Principle**:
1. Bucket lowers until bottom limit switch contacts bin (or material in bin)
2. Front motors (1&2) raise while back motors (3&4) hold position
3. Bucket rotates backward around back edge pivot point
4. Contents pour downward into bin below
5. Bin level can vary - system adapts to current fill height

---

### HOLDING Phase

#### State Machine Check
**Location**: [bucket.h:113-120](include/bucket.h#L113-L120)

```cpp
case HOLDING:
    // Check if hold time elapsed
    if (millis() - hold_start_time >= HOLD_DURATION)
    {
        bucket_state = RAISING;
        debugln("Hold complete, starting raise");
    }
    break;
```

**Algorithm**:
```
1. All motors remain stopped
2. Calculate elapsed time:
   elapsed = millis() - hold_start_time
3. If elapsed ≥ HOLD_DURATION (3000 ms):
   ├─ Set bucket_state = RAISING
   ├─ Log: "Hold complete, starting raise"
   └─ Exit (next loop will enter RAISING state)
4. Otherwise:
   └─ Continue holding (do nothing)
```

**Purpose**:
- Ensures contents have time to fully empty from bucket
- Prevents premature raising that could spill contents
- Fixed 3-second duration provides consistent operation

---

### RAISING Phase

#### Function: bucketRaise()
**Location**: [bucket.h:322-411](include/bucket.h#L322-L411)

```
Algorithm:

1. READ GYRO SENSORS
   ├─ pitch = Gyro::getPitch()  // Forward/backward tilt
   └─ roll = Gyro::getRoll()    // Left/right tilt

2. INITIALIZE MOTOR SPEEDS
   ├─ motor1_speed = RAISING_SPEED (200)
   ├─ motor2_speed = RAISING_SPEED (200)
   ├─ motor3_speed = RAISING_SPEED (200)
   └─ motor4_speed = RAISING_SPEED (200)

3. PITCH COMPENSATION (Forward/Back Leveling)
   If pitch > 5.0°:  // Front is lower than back
      ├─ motor1_speed = RAISING_SPEED × 0.7  (slow front-left)
      └─ motor2_speed = RAISING_SPEED × 0.7  (slow front-right)

   Else if pitch < -5.0°:  // Back is lower than front
      ├─ motor3_speed = RAISING_SPEED × 0.7  (slow back-left)
      └─ motor4_speed = RAISING_SPEED × 0.7  (slow back-right)

4. ROLL COMPENSATION (Left/Right Leveling)
   If roll > 5.0°:  // Right side is lower
      ├─ motor2_speed = motor2_speed × 0.7  (slow front-right, compound)
      └─ motor4_speed = motor4_speed × 0.7  (slow back-right, compound)

   Else if roll < -5.0°:  // Left side is lower
      ├─ motor1_speed = motor1_speed × 0.7  (slow front-left, compound)
      └─ motor3_speed = motor3_speed × 0.7  (slow back-left, compound)

5. PWM SAFETY CLAMPING
   ├─ motor1_speed = constrain(motor1_speed, 60, 253)
   ├─ motor2_speed = constrain(motor2_speed, 60, 253)
   ├─ motor3_speed = constrain(motor3_speed, 60, 253)
   └─ motor4_speed = constrain(motor4_speed, 60, 253)

6. SET MOTOR COMMANDS (All move up)
   ├─ motor1: cmd = "clockwise", speed = motor1_speed
   ├─ motor2: cmd = "clockwise", speed = motor2_speed
   ├─ motor3: cmd = "clockwise", speed = motor3_speed
   └─ motor4: cmd = "clockwise", speed = motor4_speed

7. CHECK INDIVIDUAL TOP LIMIT SWITCHES
   If top_limit_sw_1_val == 0 (pressed):
      ├─ motor1.cmd = "stop"
      └─ Log: "Motor 1 top limit reached"

   If top_limit_sw_2_val == 0 (pressed):
      ├─ motor2.cmd = "stop"
      └─ Log: "Motor 2 top limit reached"

   If top_limit_sw_3_val == 0 (pressed):
      ├─ motor3.cmd = "stop"
      └─ Log: "Motor 3 top limit reached"

   If top_limit_sw_4_val == 0 (pressed):
      ├─ motor4.cmd = "stop"
      └─ Log: "Motor 4 top limit reached"

8. CHECK COMPLETION (All motors at top)
   If ALL top switches pressed:
      ├─ bucket_state = IDLE
      ├─ stopAllMotors()
      ├─ Log: "Bucket fully raised - returning to IDLE"
      └─ return (exit function)

9. EXECUTE MOTOR CONTROL
   ├─ motor1Ctrl()
   ├─ motor2Ctrl()
   ├─ motor3Ctrl()
   └─ motor4Ctrl()

10. REPEAT
    └─ Function called every loop until all motors reach top
```

**Auto-Leveling During Raise**:

The raising phase uses the same auto-leveling algorithm as the platform system:

| Tilt Condition | Motors Affected | Speed Adjustment | Result |
|----------------|-----------------|------------------|--------|
| Pitch > 5° (front low) | M1, M2 | 70% | Slows front, lets back catch up |
| Pitch < -5° (back low) | M3, M4 | 70% | Slows back, lets front catch up |
| Roll > 5° (right low) | M2, M4 | 70% | Slows right side |
| Roll < -5° (left low) | M1, M3 | 70% | Slows left side |
| Diagonal tilt | 1 corner | 49% (0.7²) | Slows most-tilted corner |

**Example Speed Calculations** (RAISING_SPEED = 200):

| Bucket Position | Pitch | Roll | M1 | M2 | M3 | M4 | Notes |
|----------------|-------|------|----|----|----|----|-------|
| Level | 2° | 1° | 200 | 200 | 200 | 200 | No compensation |
| Front still low | 30° | 0° | 140 | 140 | 200 | 200 | Front slowed 30% |
| Front-right low | 20° | 8° | 140 | 98 | 200 | 140 | M2 gets both reductions |
| Nearly level | 6° | -1° | 140 | 140 | 200 | 200 | Small front tilt remains |
| Level (complete) | 1° | 0° | 200 | 200 | 200 | 200 | All equal speed |

---

## Safety Features

### 1. State-Based Limit Switch Handling
**Location**: [bucket.h:414-452](include/bucket.h#L414-L452)

```
monitorSwitches() behavior by state:

IDLE:
├─ Check bottom switches → Trigger TIPPING if any pressed
└─ Global top limit safety → Stop all if any top switch pressed

TIPPING:
└─ Only read switches (bucketTip() handles angle-based stop)

HOLDING:
└─ Only read switches (no motor control active)

RAISING:
├─ Skip monitorSwitches() processing
└─ bucketRaise() handles per-motor top limit stops
```

### 2. Per-Motor Top Limit Control
During RAISING phase:
- Each motor checks its own top limit switch
- Motors stop independently when their switch triggers
- Other motors continue until all reach top
- Prevents mechanical binding if motors reach top at different times

### 3. PWM Safety Limits
All motor speeds clamped to safe range:
- **Minimum**: 60 PWM (ensures motors don't stall)
- **Maximum**: 253 PWM (prevents excessive speed)
- Applied after all compensation calculations

### 4. Gyro-Guided Tipping
- Precise angle control prevents over-tipping
- Stops at 75° threshold (effectively 75-80° accounting for sensor accuracy)
- Avoids damage from excessive tilt

### 5. Fixed Hold Duration
- 3-second hold ensures consistent operation
- Prevents premature raising that could cause spills
- Allows complete emptying regardless of contents

---

## Configuration Parameters

### Adjustable Parameters

| Parameter | Location | Default | Description |
|-----------|----------|---------|-------------|
| **Tipping** | | | |
| TIPPING_SPEED | [bucket.h:38](include/bucket.h#L38) | 200 PWM | Speed for motors 1&2 during tipping |
| Tilt Target | [bucket.h:305](include/bucket.h#L305) | 75° | Pitch angle to stop tipping |
| **Holding** | | | |
| HOLD_DURATION | [bucket.h:37](include/bucket.h#L37) | 3000 ms | Duration to hold at tipped position |
| **Raising** | | | |
| RAISING_SPEED | [bucket.h:39](include/bucket.h#L39) | 200 PWM | Base speed for all motors during raise |
| Level Threshold | [bucket.h:335,340,347,352](include/bucket.h#L335) | 5° | Tilt angle to trigger speed compensation |
| Speed Reduction | [bucket.h:337,342,349,354](include/bucket.h#L337) | 0.7 (70%) | Speed multiplier for lower side |
| **PWM Limits** | | | |
| Minimum PWM | [bucket.h:359-362](include/bucket.h#L359-L362) | 60 | Minimum motor speed after compensation |
| Maximum PWM | [bucket.h:359-362](include/bucket.h#L359-L362) | 253 | Maximum motor speed |

### Tuning Recommendations

**If bucket tips too slowly:**
- Increase TIPPING_SPEED (try 220-250)
- Check mechanical resistance (friction, load)
- Verify motors 3&4 are fully stopped (acting as pivot)

**If bucket tips past 80° (over-tips):**
- Decrease tilt target from 75° to 70°
- Reduce TIPPING_SPEED to give more control time
- Check gyro calibration and mounting orientation

**If bucket doesn't empty completely:**
- Increase HOLD_DURATION from 3000ms to 4000-5000ms
- Verify tilt angle is actually reaching 75-80° (check serial output)
- Check mechanical setup (bucket may need steeper tilt)

**If raising is uneven (not leveling properly):**
- Decrease level threshold from 5° to 3° (more sensitive)
- Adjust speed reduction from 0.7 to 0.6 (more aggressive compensation)
- Check gyro sensor calibration
- Verify all motors have similar no-load characteristics

**If motors stall during raise (won't move):**
- Increase minimum PWM from 60 to 80
- Increase RAISING_SPEED base from 200 to 220-250
- Check mechanical load and friction
- Verify power supply can handle all 4 motors under load

---

## Operation Sequence Examples

### Example 1: Normal Tipping Cycle

```
Initial State: Bucket at mid-height, lowering

1. Bucket continues lowering via manual commands
   State: IDLE
   Motors: 1,2,3,4 all moving down

2. Bottom limit switch pressed (e.g., bottom_limit_sw_2 goes LOW)
   State: IDLE → TIPPING
   Serial: "Bottom limit triggered - starting bucket tip sequence"
   Motors: 1,2 move down at 200 PWM, 3,4 stop

3. Front edge tips forward
   State: TIPPING
   Gyro: pitch = 10° ... 20° ... 35° ... 50° ... 65° ... 75°
   Motors: 1,2 continue, 3,4 hold

4. Pitch reaches 75°
   State: TIPPING → HOLDING
   Serial: "Bucket tipped to 75.3 degrees, holding for 3 seconds"
   Motors: All stopped

5. Hold for 3 seconds (contents empty)
   State: HOLDING
   Time: 0ms ... 1000ms ... 2000ms ... 3000ms
   Motors: All stopped

6. Hold duration elapsed
   State: HOLDING → RAISING
   Serial: "Hold complete, starting raise"
   Motors: All start moving up with auto-leveling

7. Raising with auto-leveling
   State: RAISING
   Gyro: pitch = 60° ... 40° ... 20° ... 8° ... 2°
   Motors: Speed adjusts based on tilt
   Serial (periodic): "Motor 3 top limit reached"
                     "Motor 4 top limit reached"
                     "Motor 1 top limit reached"
                     "Motor 2 top limit reached"

8. All motors reach top
   State: RAISING → IDLE
   Serial: "Bucket fully raised - returning to IDLE"
   Motors: All stopped
   Ready for next cycle
```

**Duration Breakdown**:
- TIPPING: ~5-10 seconds (depends on load, speed, distance to tip)
- HOLDING: 3 seconds (fixed)
- RAISING: ~10-20 seconds (depends on height, leveling adjustments)
- **Total Cycle**: ~18-33 seconds

---

### Example 2: Tipping with Initial Left Tilt

```
Scenario: Bucket reaches bottom with slight left tilt

1. Bottom limit triggered
   State: IDLE → TIPPING
   Gyro: pitch = 2°, roll = -8° (left is lower)

2. Tipping begins
   State: TIPPING
   Motors: 1,2 move down, 3,4 hold
   Gyro: pitch increases ... 10° ... 30° ... 50° ... 75°

3. Reached 75° pitch (but still has left tilt)
   State: TIPPING → HOLDING → RAISING
   Gyro: pitch = 75°, roll = -8° (left still lower)

4. Raising with auto-leveling
   State: RAISING
   Initial compensation:
   - pitch > 5° → M1,M2 slow to 70%
   - roll < -5° → M1,M3 slow to 70%
   - M1 gets BOTH: 200 × 0.7 × 0.7 = 98 PWM
   - M2: 140 PWM, M3: 140 PWM, M4: 200 PWM

   Result: Right-back corner (M4) rises fastest
          Left-front corner (M1) rises slowest
          Platform gradually levels

5. Leveling progress
   Gyro: pitch = 60°, roll = -8°
         pitch = 40°, roll = -5° (leveling improving)
         pitch = 20°, roll = -2° (nearly level)
         pitch = 4°, roll = 0° (level achieved)

6. All motors reach top (now level)
   State: RAISING → IDLE
   Serial: "Bucket fully raised - returning to IDLE"
```

**Key Point**: Auto-leveling during raising compensates for any initial tilt, ensuring bucket returns to level position regardless of tipping unevenness.

---

### Example 3: Partial Sequence Interruption

```
Scenario: System reset or power loss during tipping sequence

1. Bucket in TIPPING state, pitch = 45°
   → Power loss or reset

2. System reboots
   State: IDLE (bucket_state resets to IDLE)
   Physical position: Bucket still at 45° tilt
   Problem: Bucket stuck at 45°, not level

Manual Recovery:
3. Send manual motor commands:
   Motor3: cmd=anticlockwise,pwm=150  (lower back to level)
   Motor4: cmd=anticlockwise,pwm=150  (lower back to level)

4. Manually level the bucket to ~0° pitch

5. Trigger new sequence:
   Lower bucket until bottom limit triggers
   → Normal tipping sequence resumes

Automatic Recovery (Future Enhancement):
- On init(), check gyro pitch angle
- If |pitch| > 15°, enter RAISING state immediately
- Auto-level and raise to top position
- Return to IDLE when complete
```

**Safety Note**: Current implementation does not have automatic crash recovery. Operator must manually level bucket after unexpected interruption.

---

## Troubleshooting

### Problem: Bucket doesn't tip when bottom limit pressed

**Possible Causes:**
- Not in IDLE state (stuck in another state)
- Bottom limit switch wiring issue
- Switch not actually being pressed (mechanical misalignment)

**Solutions:**
- Check serial output for state transitions
- Verify switch wiring: should read 1 normally, 0 when pressed
- Test switch manually: check `bottom_limit_sw_N_val` in logSwitchStates()
- Reset Arduino to force IDLE state

---

### Problem: Bucket tips past 80° (over-tips)

**Possible Causes:**
- Gyro not reading correctly
- Tilt threshold too high
- TIPPING_SPEED too fast (momentum carries it past threshold)

**Solutions:**
- Check gyro I2C connection and calibration
- Reduce tilt target from 75° to 70° in [bucket.h:305](include/bucket.h#L305)
- Reduce TIPPING_SPEED from 200 to 150 PWM
- Add serial debug to print pitch angle during tipping

---

### Problem: Bucket doesn't tip far enough (stops at 50-60°)

**Possible Causes:**
- Gyro orientation wrong (pitch sign reversed)
- Mechanical obstruction preventing full tilt
- Motors 1&2 not strong enough / load too heavy

**Solutions:**
- Check gyro mounting and orientation
- Swap pitch sign: change `if (pitch >= 75.0)` to `if (pitch <= -75.0)`
- Increase TIPPING_SPEED from 200 to 240 PWM
- Check mechanical system for binding or obstruction

---

### Problem: Bucket doesn't hold for 3 seconds (raises immediately)

**Possible Causes:**
- HOLDING state not reached (skipped)
- Timer overflow issue
- State machine logic error

**Solutions:**
- Check serial output: should see "holding for 3 seconds" message
- Verify `hold_start_time = millis()` is being set
- Add debug logging in HOLDING case to print elapsed time
- Check if `millis()` has overflowed (rare, occurs after 49 days uptime)

---

### Problem: Bucket raises unevenly (not leveling)

**Possible Causes:**
- Gyro not updating (check main.cpp loop)
- Level threshold too insensitive (5° is too large)
- Motor speed differences overwhelming compensation
- Mechanical binding on one corner

**Solutions:**
- Verify gyro updates enabled in [main.cpp:37-41](../src/main.cpp#L37-L41)
- Reduce level threshold from 5° to 3° in [bucket.h:335,340](include/bucket.h#L335)
- Increase speed reduction from 0.7 to 0.6 (more aggressive)
- Check mechanical system for friction differences
- Manually test each motor at same PWM to verify similar speeds

---

### Problem: One or more motors don't stop at top limit

**Possible Causes:**
- Top limit switch wiring issue
- Switch mechanically failed
- RAISING state not active (monitorSwitches() taking over)

**Solutions:**
- Test switches: check `top_limit_sw_N_val` in logSwitchStates()
- Verify INPUT_PULLUP mode: should read 1 normally, 0 when pressed
- Check state machine: ensure bucket_state == RAISING
- Add debug logging in bucketRaise() per-motor stop section

---

### Problem: Motors stall or won't move during raise

**Possible Causes:**
- PWM too low after compensation (below motor starting threshold)
- Power supply voltage drop under load
- Mechanical binding or excessive friction

**Solutions:**
- Increase minimum PWM from 60 to 80 in [bucket.h:359](include/bucket.h#L359)
- Increase RAISING_SPEED from 200 to 230
- Check power supply: should provide adequate current for 4 motors
- Reduce mechanical load or improve lubrication

---

## Testing Checklist

### Initial Safety Testing
- [ ] Verify all 8 limit switches read correctly (1 = open, 0 = pressed)
- [ ] Test manual motor control (Motor1-4 commands work in IDLE)
- [ ] Confirm gyro readings are reasonable (±10° when tilted by hand)
- [ ] Ensure emergency stop (reset button) is accessible

### Tipping Sequence Testing
- [ ] Trigger sequence by pressing any bottom limit switch
- [ ] Verify only motors 1&2 move during TIPPING (3&4 stay stopped)
- [ ] Confirm pitch angle reaches 75-80° (check serial output)
- [ ] Verify transition to HOLDING state with serial message
- [ ] Confirm 3-second hold (time with stopwatch)
- [ ] Verify automatic transition to RAISING after hold

### Raising and Leveling Testing
- [ ] Confirm all 4 motors move upward during RAISING
- [ ] Verify auto-leveling: tilt bucket manually, observe speed differences
- [ ] Test per-motor top limit stops: each motor stops independently
- [ ] Verify "Bucket fully raised" message when all motors at top
- [ ] Confirm return to IDLE state after raising complete

### Edge Case Testing
- [ ] Test with bucket initially tilted left/right (roll ≠ 0)
- [ ] Test with uneven load distribution
- [ ] Trigger sequence multiple times in succession
- [ ] Test state recovery after manual motor commands during sequence
- [ ] Verify behavior if only one bottom switch triggers

### Performance Tuning
- [ ] Measure total cycle time (target: 18-33 seconds)
- [ ] Verify bucket empties completely during 3-second hold
- [ ] Check maximum tilt during tipping (should not exceed 85°)
- [ ] Verify bucket returns to level position (±2° final tilt acceptable)
- [ ] Monitor for oscillation or hunting during leveling

---

## Future Enhancements

### 1. Adjustable Tilt Angle via Serial Command
```cpp
Bucket: cmd=set_angle,value=80
```
- Allow runtime adjustment of tilt target (60-85° range)
- Store in EEPROM for persistence across reboots
- Useful for different materials requiring different dump angles

### 2. Variable Hold Duration
```cpp
Bucket: cmd=set_hold,value=5000
```
- Adjust hold time based on contents (light vs heavy materials)
- Range: 1000-10000 ms (1-10 seconds)
- Auto-adjust based on load sensor (future hardware addition)

### 3. Load Sensing and Adaptive Speed
- Add load cells or current sensors to detect bucket weight
- Adjust TIPPING_SPEED and RAISING_SPEED based on load
- Slower speeds for heavy loads (more control)
- Faster speeds for light loads (efficiency)

### 4. Crash Recovery on Boot
```cpp
In init():
  float pitch = Gyro::getPitch()
  if (abs(pitch) > 15°):
    bucket_state = RAISING  // Auto-recover
    debugln("Crash recovery: Raising bucket to level position")
```
- Detect non-level position on startup
- Automatically raise and level without manual intervention
- Log recovery event for diagnostics

### 5. State Logging to SD Card
- Log all state transitions with timestamps
- Record gyro angles at key points
- Track cycle times and performance metrics
- Useful for diagnostics and optimization

### 6. Remote Monitoring via WiFi/Bluetooth
- Transmit bucket state and position to remote display
- Alert operator when tipping sequence starts
- Send notifications on completion or errors
- Enable remote manual override

### 7. Multi-Bucket Coordination
- Coordinate multiple bucket systems
- Stagger tipping sequences to balance power load
- Synchronized operation for production line efficiency

### 8. Predictive Maintenance
- Track motor run times and cycle counts
- Monitor for performance degradation
- Alert when maintenance due (lubrication, belt tension, etc.)
- Log anomalies (excessive tilt, slow speeds, high current)

---

## Conclusion

The bucket tipping system provides a reliable, automated solution for emptying suspended buckets at the bottom of their travel. The system prioritizes safety and predictability through:

1. **State Machine Control**: Clear separation of tipping phases prevents conflicts and ensures deterministic behavior
2. **Gyro-Guided Tipping**: Precise angle control ensures consistent dump angle regardless of load variations
3. **Fixed Hold Duration**: 3-second hold ensures complete emptying for all materials
4. **Auto-Leveling Raise**: Smooth return to level position prevents spillage and mechanical stress
5. **Per-Motor Limit Switches**: Independent motor stops prevent binding and ensure safe operation
6. **PWM Safety Limits**: 60-253 PWM range ensures reliable motor operation without stalling or overspeeding

### Key Advantages:

✅ **Fully Automatic**: No operator intervention required after trigger
✅ **Consistent Operation**: Same tipping angle and hold time every cycle
✅ **Safe**: Multiple safety features prevent over-tipping and mechanical damage
✅ **Adaptive**: Auto-leveling compensates for load distribution and mechanical variations
✅ **Configurable**: Key parameters easily adjustable for different applications
✅ **Reliable**: State machine prevents logic conflicts and race conditions

### Typical Applications:

- **Material Handling**: Automated dumping of bulk materials from elevated buckets
- **Waste Management**: Tipping garbage or recycling bins at collection points
- **Agriculture**: Grain elevators, feed hoppers, produce handling
- **Manufacturing**: Part bins, scrap collection, material transfer
- **Mining**: Ore buckets, tailings disposal, material transport

This implementation provides a robust foundation that can be adapted to various bucket tipping applications with minimal modifications to the core algorithm.
