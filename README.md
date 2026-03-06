# MCU Firmware — RVM Platform Controller

**Target:** ATmega1284P @ 8MHz internal oscillator
**Framework:** Arduino (MightyCore)
**Toolchain:** PlatformIO

---

## Hardware Pin Mapping

| Pin | Function |
|-----|----------|
| 9   | ENA — conveyor motor PWM enable |
| 18  | IN2 — conveyor motor direction |
| 19  | IN1 — conveyor motor direction |
| 20  | motorDownPin — platform down |
| 21  | motorUpPin — platform up |
| 22  | HC-SR05 trigger |
| 23  | HC-SR05 echo |
| 24  | Service door switch |
| 25  | Left door switch |
| 26  | Right door switch |
| 27  | Top limit switch |
| 28  | Bottom limit switch |

---

## Serial Protocol

**Port:** Serial1 (UART0), 9600 baud
Commands are newline-terminated (`\n`). Responses are printed back on Serial1.

| Command | Response |
|---------|----------|
| `forward` | `Moving forward` |
| `back` | `Moving backward` |
| `stop` | `Stopped` |
| `hc-level` | `hc-level:<distance_cm>` |
| `doors` | `door-status:service:<0/1>,left:<0/1>,right:<0/1>` |
| `platform-up` | `platform: moving up` |
| `platform-down` | `platform: moving down` |
| `platform-stop` | `platform: stopped` |
| `platform-status` | `platform-status:<state>,ref:<cm>,top:<0/1>,bottom:<0/1>` |

Platform states: `idle`, `moving_up`, `moving_down`, `dead_time`

---

## Platform Control Logic

The platform uses a **non-blocking state machine** (`PlatformControl::update()` called every loop).

### Auto-compensation
- Reference distance defaults to `15.0 cm` (hardcoded in `platform_control.h`)
- When `distance <= referenceDistance` (ullage drops), platform lowers by `COMPENSATION_STEP = 5 cm`
- Stops when ullage is restored (`distance >= compensationTarget`)

### Manual mode
- Triggered by `platform-up` / `platform-down` serial commands
- No auto-stop — user must send `platform-stop`

### Safety
- Top and bottom limit switches stop the motor immediately
- Direction reversals enforce a **1-second dead time** (`DEAD_TIME_MS = 1000`) to protect the motor driver

---

## Upload

### Requirements
- PlatformIO installed: `~/.platformio/penv/bin/pio`
- USBtiny programmer connected (VID `0x1781`, PID `0x0c9f`)
- USB permissions set:

```bash
sudo usermod -a -G plugdev $USER
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="1781", ATTR{idProduct}=="0c9f", GROUP="plugdev", MODE="0666"' | sudo tee /etc/udev/rules.d/99-usbtiny.rules
sudo udevadm control --reload-rules && sudo udevadm trigger
```

### Upload command

```bash
cd /home/finplus/RVM/mcu_test_pub
/home/finplus/.platformio/penv/bin/pio run -e Upload_USBtiny -t upload
```

Or via UART (requires bootloader on COM4 / `/dev/ttyUSB0`):

```bash
/home/finplus/.platformio/penv/bin/pio run -e Upload_UART -t upload
```

### Upload flags (`Upload_USBtiny`)
- `-B 10` — ISP bit clock delay (μs), safe for 8MHz internal oscillator
- `-V` — skip flash verification (USBtiny known read-back issue at addr `0x004b`)

---

## Build size (current)

| Memory | Used | Total | % |
|--------|------|-------|---|
| Flash  | 8966 B | 131072 B | 6.8% |
| RAM    | 883 B  | 16384 B  | 5.4% |

---

## File Structure

```
mcu_test_pub/
├── platformio.ini
├── src/
│   ├── main.cpp              # Setup, loop, serial command handler
│   ├── motor_control.cpp     # Conveyor: forward / back / stop
│   ├── platform_control.cpp  # Platform state machine + auto-compensation
│   ├── hc_sr05.cpp           # HC-SR05 ultrasonic sensor (non-blocking)
│   ├── door_interrupt.cpp    # Door state debounce
│   └── level_sensor.cpp      # (unused in current branch)
└── include/
    ├── platform_control.h
    ├── hc_sr05.h
    ├── motor_control.h
    └── door_state.h
```

---

## Branch: `feature/platform-control-hc`

Changes from main:
- Removed `I2cLevelSensor` — platform now uses **HC-SR05 only** for distance
- `PlatformControl` constructor updated: `(topPin, bottomPin, upPin, downPin, HcSr05*)`
- `getSensorDistance()` returns HC-SR05 reading directly (no sensor averaging)
- Auto-compensation trigger changed: fires when `distance <= referenceDistance` (not threshold-based)
- `compensationTarget` replaces `referenceSet` pattern — lowers exactly `COMPENSATION_STEP` cm per step
- Limit switch handling unified: both top and bottom checked in single block
