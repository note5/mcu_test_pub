#include "platform_control.h"

PlatformControl::PlatformControl(uint8_t topPin, uint8_t bottomPin,
                                 uint8_t upPin, uint8_t downPin,
                                 HcSr05 *hc)
    : topLimitPin(topPin), bottomLimitPin(bottomPin),
      motorUpPin(upPin), motorDownPin(downPin),
      hcSensor(hc),
      state(IDLE), pendingState(IDLE), deadTimeStart(0), compensationTarget(0),
      moveStartTime(0), lastCheckTime(0) {}

// Null-safe wrappers — return safe defaults when no sensor is attached
// (allows testing platform motor logic without a connected HC-SR05)
float PlatformControl::getSensorMinDistance()
{
    return hcSensor ? hcSensor->getMinDistance() : -1.0;
}

bool PlatformControl::allSensorsBelow(float threshold)
{
    return hcSensor ? hcSensor->allBelow(threshold) : false;
}

void PlatformControl::begin()
{
    // Limit switch pins are INPUT (external pull-down assumed on the PCB)
    pinMode(topLimitPin, INPUT);
    pinMode(bottomLimitPin, INPUT);
    pinMode(motorUpPin, OUTPUT);
    pinMode(motorDownPin, OUTPUT);
    stop();   // ensure motor is off at power-on
}

/**
 * Main state machine — call every loop() iteration.
 *
 * Flow:
 *  1. Check limit switches first (safety override, runs every cycle)
 *  2. Then execute current state logic:
 *     - IDLE: periodically check if bin is full enough to trigger lowering
 *     - MOVING_DOWN: monitor ullage until compensation target is reached
 *     - MOVING_UP: manual only, no auto-stop (user sends platform-stop)
 *     - DEAD_TIME: wait 1s then engage the pending direction
 */
void PlatformControl::update()
{
    float distance = getSensorMinDistance();

    // --- Limit switch safety --- checked every cycle, regardless of state ---
    // Only stop when the motor is driving TOWARD the active limit, so the
    // user can still reverse away from a triggered limit switch.
    if (isBottomLimit() && state == MOVING_DOWN)
    {
        Serial1.println("platform: bottom limit, stopping");
        stop();
    }
    else if (isTopLimit() && state == MOVING_UP)
    {
        Serial1.println("platform: top limit, stopping");
        stop();
    }
    else if ((isBottomLimit() || isTopLimit()) && state == DEAD_TIME)
    {
        // Cancel the pending move — no point reversing into a limit
        Serial1.println("platform: limit switch, cancelling pending move");
        state = IDLE;
    }

    switch (state)
    {
    case IDLE:
        // Throttled compensation check — only runs every CHECK_INTERVAL_MS (10s)
        // to avoid reacting to transient sensor spikes
        if (!manualMode && (millis() - lastCheckTime >= CHECK_INTERVAL_MS))
        {
            lastCheckTime = millis();
            // Trigger only when ALL sensors agree the bin is full past the
            // reference distance — a single high reading isn't enough
            if (allSensorsBelow(referenceDistance))
            {
                // Target: current distance + 5cm. As platform lowers,
                // ullage increases until it reaches this value.
                compensationTarget = distance + COMPENSATION_STEP;
                moveStartTime = millis();
                Serial1.print("platform: lowering 5cm (min ullage ");
                Serial1.print(distance);
                Serial1.println("cm)");
                moveDown();
            }
        }
        break;

    case MOVING_DOWN:
        // Auto-stop: sensor target reached OR time safety cap exceeded
        if (!manualMode)
        {
            if (distance > 0 && distance >= compensationTarget)
            {
                Serial1.println("platform: compensation done, stopping");
                stop();
            }
            else if (millis() - moveStartTime >= MAX_MOVE_MS)
            {
                Serial1.println("platform: max move time reached, stopping");
                stop();
            }
        }
        break;

    case MOVING_UP:
        // No auto-stop — up movement is always manual (maintenance/reset).
        // User must send "platform-stop" via serial.
        break;

    case DEAD_TIME:
        // Wait for DEAD_TIME_MS before engaging the new direction.
        // Both motor pins are LOW during this interval.
        if (millis() - deadTimeStart >= DEAD_TIME_MS)
        {
            if (pendingState == MOVING_UP)
            {
                digitalWrite(motorUpPin, HIGH);
                digitalWrite(motorDownPin, LOW);
                state = MOVING_UP;
                Serial1.println("platform: moving up");
            }
            else if (pendingState == MOVING_DOWN)
            {
                digitalWrite(motorUpPin, LOW);
                digitalWrite(motorDownPin, HIGH);
                state = MOVING_DOWN;
                Serial1.println("platform: moving down");
            }
            else
            {
                state = IDLE;
            }
        }
        break;
    }
}

void PlatformControl::setReference(float ref)
{
    referenceDistance = ref;
}

float PlatformControl::getReference()
{
    return referenceDistance;
}

bool PlatformControl::isTopLimit()
{
    return digitalRead(topLimitPin) == HIGH;
}

bool PlatformControl::isBottomLimit()
{
    return digitalRead(bottomLimitPin) == HIGH;
}

/**
 * Drive platform up. If currently moving down, enters DEAD_TIME first
 * to let the motor coast to a stop before reversing. Ignores the command
 * entirely if already in DEAD_TIME (let the pending move complete).
 */
void PlatformControl::moveUp()
{
    if (isTopLimit()) return;
    if (state == MOVING_DOWN)
    {
        digitalWrite(motorUpPin, LOW);
        digitalWrite(motorDownPin, LOW);
        pendingState = MOVING_UP;
        deadTimeStart = millis();
        state = DEAD_TIME;
        Serial1.println("platform: dead time before direction change");
        return;
    }
    if (state == DEAD_TIME) return;   // don't interrupt an in-progress reversal
    digitalWrite(motorUpPin, HIGH);
    digitalWrite(motorDownPin, LOW);
    state = MOVING_UP;
}

/**
 * Drive platform down. Same dead-time reversal logic as moveUp().
 * Called both by serial command ("platform-down") and by the auto-
 * compensation logic in update().
 */
void PlatformControl::moveDown()
{
    if (isBottomLimit()) return;
    if (state == MOVING_UP)
    {
        digitalWrite(motorUpPin, LOW);
        digitalWrite(motorDownPin, LOW);
        pendingState = MOVING_DOWN;
        deadTimeStart = millis();
        state = DEAD_TIME;
        Serial1.println("platform: dead time before direction change");
        return;
    }
    if (state == DEAD_TIME) return;
    digitalWrite(motorUpPin, LOW);
    digitalWrite(motorDownPin, HIGH);
    state = MOVING_DOWN;
}

// Immediate stop — both pins LOW, state reset to IDLE.
// Also cancels any pending dead-time transition.
void PlatformControl::stop()
{
    digitalWrite(motorUpPin, LOW);
    digitalWrite(motorDownPin, LOW);
    state = IDLE;
}

const char* PlatformControl::getStateName()
{
    switch (state)
    {
    case IDLE:         return "idle";
    case MOVING_DOWN:  return "moving_down";
    case MOVING_UP:    return "moving_up";
    case DEAD_TIME:    return "dead_time";
    default:           return "unknown";
    }
}
