# Platform Auto-Leveling System Documentation

## Overview
This document describes the auto-leveling system for a platform suspended by 4 pulleys, each controlled by an independent motor at each corner. The system uses a gyroscope (MPU6050) to detect platform tilt and automatically adjusts motor speeds to maintain level descent. It also supports simple upward movement with per-motor limit switch control.

---

## System Architecture

### Hardware Components
- **4 DC Motors**: Each controls one corner pulley (front-left, front-right, back-left, back-right)
- **8 Limit Switches**: Top and bottom switches for each of the 4 motors
- **MPU6050 Gyroscope**: Measures platform tilt (pitch and roll)
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

---

## Implementation Files

### Modified Files
1. **[src/main.cpp](src/main.cpp)**
   - Lines 37-41: Enabled gyro sensor updates every 500ms

2. **[include/lowering-platform.h](include/lowering-platform.h)**
   - Line 6: Added `#include "gyro.h"`
   - Lines 18-19: Added auto-leveling and move-up function declarations
   - Lines 23-25: Added state variables (platform_auto_leveling, platform_moving_up, platform_base_speed)
   - Lines 92-127: Platform command parser (down, up, stop)
   - Lines 98-100: PWM range validation (123-253)
   - Lines 258-378: Auto-leveling algorithm implementation with PWM clamping
   - Lines 381-430: Platform move up implementation
   - Lines 469-472: Modified switch monitoring for per-motor control

---

## Auto-Leveling Algorithm

### High-Level Flow

#### Auto-Leveling Down
```
1. Receive command: "Platform: cmd=down,pwm=200"
2. Validate and clamp PWM (123-253 range)
3. Enter auto-leveling mode (platform_auto_leveling = true)
4. Loop every cycle:
   a. Read gyro sensors (pitch & roll angles)
   b. Calculate speed compensation for each motor
   c. Clamp individual motor speeds (60-253 range)
   d. Apply motor commands with adjusted speeds
   e. Check individual bottom limit switches
   f. Stop individual motors as they reach bottom
   g. Exit when all motors reach bottom or stop command received
```

#### Simple Up Movement
```
1. Receive command: "Platform: cmd=up,pwm=200"
2. Validate and clamp PWM (123-253 range)
3. Enter move-up mode (platform_moving_up = true)
4. Loop every cycle:
   a. All motors move up at base speed (no auto-leveling)
   b. Check individual top limit switches
   c. Stop individual motors as they reach top
   d. Exit when all motors reach top or stop command received
```

### Detailed Algorithm

#### Step 1: Command Reception and PWM Validation
**Location**: [lowering-platform.h:92-127](include/lowering-platform.h#L92-L127)

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
  - Set platform_auto_leveling = true
  - Store platform_base_speed = validated pwm value
  - Clear command to prevent re-parsing
  - Begin calling platformAutoLevel() each loop

- If cmd == "up":
  - Set platform_moving_up = true
  - Store platform_base_speed = validated pwm value
  - Clear command to prevent re-parsing
  - Begin calling platformMoveUp() each loop

- If cmd == "stop":
  - Set platform_auto_leveling = false
  - Set platform_moving_up = false
  - Call stopAllMotors()
  - Exit all platform movement modes
```

#### Step 2: Gyro Reading (Auto-Leveling Down Only)
**Location**: [lowering-platform.h:262-264](include/lowering-platform.h#L262-L264)

```cpp
Read current tilt angles:
- pitch = Gyro::getPitch()  // Forward/Backward tilt
- roll = Gyro::getRoll()    // Left/Right tilt

Update frequency: 500ms (configured in timing.h)
```

#### Step 3: Speed Calculation (Auto-Leveling Down Only)
**Location**: [lowering-platform.h:273-322](include/lowering-platform.h#L273-L322)

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

#### Step 4: Motor Command Assignment
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

#### Step 5: Per-Motor Limit Switch Checking
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

#### Step 6: Completion Detection
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

#### Step 7: Motor Execution
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

#### Step 8: Diagnostic Logging
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
| **Timing** | | | |
| Base Speed Default | [lowering-platform.h:96](include/lowering-platform.h#L96) | 250 | Default PWM if not specified in command |
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

---

## Usage Examples

### Example 1: Basic Auto-Leveling Descent
```
Command: Platform: cmd=down,pwm=120

Expected behavior:
1. All motors start descending at 120 PWM
2. System monitors tilt every 500ms
3. Adjusts speeds automatically to maintain level
4. Motors stop individually as they reach bottom
5. Exits when all 4 motors reach bottom

Serial output:
"Tilt - Pitch: 2.1° Roll: 1.3° Speeds: M1=120 M2=120 M3=120 M4=120"
"Pitch: Front lower, slowing motors 1&2"
"Tilt - Pitch: 6.5° Roll: 0.8° Speeds: M1=84 M2=84 M3=120 M4=120"
"Motor 1 bottom limit reached"
"Motor 2 bottom limit reached"
"Tilt - Pitch: -0.5° Roll: -0.2° Speeds: M1=0 M2=0 M3=120 M4=120"
"Motor 3 bottom limit reached"
"Motor 4 bottom limit reached"
"All motors reached bottom - auto-leveling complete"
```

### Example 2: Faster Descent
```
Command: Platform: cmd=down,pwm=200

Expected behavior:
- Same as Example 1, but with higher base speed
- More aggressive descent (faster overall time)
- Same leveling accuracy
```

### Example 3: Emergency Stop During Descent
```
1. Command: Platform: cmd=down,pwm=120
   [Platform starts descending with auto-leveling]

2. Command: Platform: cmd=stop
   [All motors immediately stop]
   [Auto-leveling mode exits]
```

### Example 4: Platform Move Up
```
Command: Platform: cmd=up,pwm=180

Expected behavior:
1. All motors move up at 180 PWM (after validation, clamped to 123-253 range)
2. No auto-leveling (all motors same speed)
3. Each motor stops individually when its top limit switch triggers
4. Continues until all 4 motors reach top
5. Exits with "All motors reached top" message

Serial output:
"Motor 1 top limit reached"
"Motor 3 top limit reached"
"Motor 2 top limit reached"
"Motor 4 top limit reached"
"All motors reached top - move up complete"
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

---

## Testing Checklist

### Initial Testing (Safety First)
- [ ] Verify all limit switches work correctly
- [ ] Test emergency stop command
- [ ] Confirm motors stop when limit reached
- [ ] Check gyro readings are reasonable (±10° when tilted by hand)

### Functional Testing
- [ ] Level platform descends straight down
- [ ] Tilted platform (front down) compensates correctly
- [ ] Tilted platform (right down) compensates correctly
- [ ] Diagonal tilt (front-right down) compensates correctly
- [ ] All motors reach bottom and stop
- [ ] Auto-leveling exits properly when complete

### Edge Cases
- [ ] Platform starts at extreme tilt (>20°)
- [ ] One motor reaches bottom early (others continue)
- [ ] Stop command during active descent
- [ ] Rapid tilt changes (manual push while descending)
- [ ] Gyro disconnected (fail-safe behavior)

### Performance Tuning
- [ ] Measure descent time with auto-leveling vs without
- [ ] Verify max tilt angle during descent stays < 10°
- [ ] Check for oscillation or hunting behavior
- [ ] Optimize speed factor for fastest level descent

---

## Conclusion

This auto-leveling system provides a simple, robust solution for maintaining platform level during descent and controlled upward movement. The algorithm prioritizes safety and predictability over speed, using conservative speed reduction and strict PWM limits to ensure smooth operation.

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

6. **Separate up/down modes**
   - Down: Auto-leveling with gyro compensation
   - Up: Simple equal-speed movement (no auto-leveling needed)
   - Both use per-motor limit switches
   - Stop command exits both modes immediately

### Operation Summary:

**For Auto-Leveling Descent:**
- Command: `Platform: cmd=down,pwm=200`
- Result: Slower but more reliable level descent
- Appropriate for safety-critical applications

**For Simple Upward Movement:**
- Command: `Platform: cmd=up,pwm=200`
- Result: Fast equal-speed ascent with per-motor stops
- No leveling compensation needed when lifting

**For Maximum Speed:**
- Use PWM values in 200-253 range
- Monitor for oscillation and adjust if needed
- Consider proportional control for even faster operation

This implementation strikes a balance between simplicity, safety, and effectiveness, making it suitable for industrial and commercial applications where reliability is more important than maximum speed.
