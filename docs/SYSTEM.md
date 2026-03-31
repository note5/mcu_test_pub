# Finplus RVM — Embedded Platform Controller

## Overview

Firmware for the ATmega1284P (8MHz internal oscillator) that controls a Reverse Vending Machine (RVM) platform. As bottles fill the bin, the platform automatically lowers to maintain a consistent drop height for the user.

Communicates with a Web Serial dashboard (`manual.html`) over UART1 at 9600 baud.

## Hardware

| Component | Description |
|-----------|-------------|
| MCU | ATmega1284P @ 8MHz (MightyCore, internal oscillator) |
| Programmer | USBtiny ISP (`-B 10` flag for reliable clock) |
| Conveyor motor | L298N H-bridge (ENA/IN1/IN2) |
| Platform motor | Relay or H-bridge pair (UP/DOWN pins) |
| Ultrasonic sensors | 5x HC-SR05 (shared trigger, individual echo) |
| Door sensors | 3x magnetic reed switches (service, left, right) |
| Limit switches | 2x mechanical (top, bottom) for platform safety |

## Pin Map

| Pin | Function | Direction |
|-----|----------|-----------|
| 0 | HC-SR05 echo — corner 1 | INPUT |
| 1 | HC-SR05 echo — corner 2 | INPUT |
| 2 | HC-SR05 echo — corner 3 | INPUT |
| 3 | HC-SR05 echo — corner 4 | INPUT |
| 9 | ENA — conveyor motor PWM | OUTPUT |
| 18 | IN2 — conveyor motor direction | OUTPUT |
| 19 | IN1 — conveyor motor direction | OUTPUT |
| 20 | Platform motor DOWN | OUTPUT |
| 21 | Platform motor UP | OUTPUT |
| 22 | HC-SR05 shared trigger | OUTPUT |
| 23 | HC-SR05 echo — center | INPUT |
| 24 | Service door reed switch | INPUT |
| 25 | Left door reed switch | INPUT |
| 26 | Right door reed switch | INPUT |
| 30 | Bottom limit switch | INPUT |
| 31 | Top limit switch | INPUT |

## Ultrasonic Sensor Array

Five HC-SR05 sensors cover the bin surface — one at the center and one at each corner:

```
Corner1 (pin 0)     Corner2 (pin 1)
         Center (pin 23)
Corner3 (pin 2)     Corner4 (pin 3)
```

All sensors share trigger pin 22. They are read sequentially in round-robin fashion — one sensor per second — to avoid crosstalk. A full sweep takes 5 seconds.

A reading of `-1.0` means no valid echo has been received from that sensor yet.

## Platform Control State Machine

```
IDLE ──(all 5 sensors <= referenceDistance)──> MOVING_DOWN
MOVING_DOWN ──(min ullage >= compensationTarget)──> IDLE
MOVING_DOWN ──(MAX_MOVE_MS elapsed)──> IDLE
MOVING_DOWN ──(bottom limit hit)──> IDLE
MOVING_UP ──(top limit hit)──> IDLE
Any ──(direction reversal)──> DEAD_TIME ──(1s)──> target state
```

### Key Parameters (compile-time, set in `platform_control.h`)

| Parameter | Default | Description |
|-----------|---------|-------------|
| `referenceDistance` | 10.0 cm | Ullage threshold that triggers compensation |
| `COMPENSATION_STEP` | 5.0 cm | How far the platform lowers per step |
| `DEAD_TIME_MS` | 1000 ms | Pause before motor direction reversal |
| `CHECK_INTERVAL_MS` | 10000 ms | How often the IDLE state checks if compensation is needed |
| `MAX_MOVE_MS` | 1000 ms | Safety cap — motor stops after this time regardless of sensor readings |
| `manualMode` | false | Set to true to disable auto-compensation |

### Compensation Logic

1. Every 10 seconds (while IDLE), the controller reads all 5 sensors
2. If **all** sensors report a distance at or below `referenceDistance`, the bin is considered uniformly full
3. The platform lowers until the minimum sensor reading increases by `COMPENSATION_STEP` (5 cm), or `MAX_MOVE_MS` (1s) elapses — whichever comes first
4. Limit switches override everything — the motor stops immediately if the platform hits a physical limit

### Safety

- Limit switches are checked every loop iteration, regardless of state
- The motor only stops when moving **toward** an active limit (so you can still reverse away)
- Dead time prevents H-bridge shoot-through on rapid direction changes
- A pending dead-time move is cancelled if a limit switch activates during the pause

## Conveyor Motor

An L298N H-bridge drives a conveyor belt that moves bottles into the bin.

| Pin | Function |
|-----|----------|
| 9 (ENA) | PWM speed control (set to 255 = full speed) |
| 19 (IN1) | Direction pin 1 |
| 18 (IN2) | Direction pin 2 |

| Command | IN1 | IN2 | ENA | Result |
|---------|-----|-----|-----|--------|
| `forward` | HIGH | LOW | 255 | Belt moves forward |
| `back` | LOW | HIGH | 255 | Belt moves backward |
| `stop` | LOW | LOW | 0 | Brake (both LOW = coast stop) |

Controlled manually via serial commands only — no automatic logic.

## Serial Commands

Sent as newline-terminated strings over UART1.

| Command | Response |
|---------|----------|
| `hc-level` | `hc-level:12.3,14.5,-1.00,-1.00,-1.00` (5 sensor readings) |
| `doors` | `door-status:service:0,left:1,right:0` |
| `platform-up` | Moves platform up (manual, no auto-stop) |
| `platform-down` | Moves platform down (manual or auto) |
| `platform-stop` | Stops platform motor |
| `platform-status` | `platform-status:idle,ref:10.00,top:0,bottom:1` |
| `forward` | Conveyor motor forward |
| `back` | Conveyor motor backward |
| `stop` | Conveyor motor stop |

## Auto-Report

The MCU pushes `hc-level:` and `platform-status:` lines every 2 seconds without being asked. The Web Serial dashboard (`manual.html`) parses these to update the UI.

## Dashboard (manual.html)

Single-file HTML dashboard using the Web Serial API. No server required — open directly in Chrome/Edge.

- Click the connection bar to pair with the MCU
- Shows all 5 sensor readings in a spatial layout (corners + center)
- Platform state, reference distance, and limit switch indicators
- Up/Down/Stop buttons for manual control
- Serial log at the bottom

## Building & Uploading

```bash
# Build
pio run -e Upload_USBtiny

# Build and upload
pio run -e Upload_USBtiny -t upload

# If upload verification fails, burn fuses first:
pio run -e fuses_bootloader -t bootloader
```

## Project Structure

```
include/
  hc_sr05.h            — Multi-sensor ultrasonic driver
  platform_control.h   — Platform state machine
  motor_control.h      — Conveyor motor (L298N)
  door_state.h         — Door reed switch with debounce
src/
  main.cpp             — Setup, loop, auto-report, serial commands
  hc_sr05.cpp          — Round-robin sensor reading
  platform_control.cpp — Compensation logic, limit switches, dead time
  motor_control.cpp    — Forward/back/stop for conveyor
  door_interrupt.cpp   — Debounced door state tracking
manual.html            — Web Serial dashboard
platformio.ini         — Build configuration
```
