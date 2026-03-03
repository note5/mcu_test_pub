#include "platform_control.h"

PlatformControl::PlatformControl(uint8_t topPin, uint8_t bottomPin,
                                 uint8_t upPin, uint8_t downPin,
                                 I2cLevelSensor *i2c, HcSr05 *hc)
    : topLimitPin(topPin), bottomLimitPin(bottomPin),
      motorUpPin(upPin), motorDownPin(downPin),
      i2cSensor(i2c), hcSensor(hc), referenceDistance(0),
      referenceSet(false), state(IDLE), pendingState(IDLE), deadTimeStart(0) {}

// returns average if both valid, single if only one valid, -1 if none
float PlatformControl::getSensorDistance()
{
    float i2cDist = i2cSensor ? i2cSensor->getDistance() : -1.0;
    float hcDist = hcSensor ? hcSensor->getDistance() : -1.0;

    if (i2cDist > 0 && hcDist > 0)
        return (i2cDist + hcDist) / 2.0;
    if (i2cDist > 0)
        return i2cDist;
    if (hcDist > 0)
        return hcDist;
    return -1.0;
}

void PlatformControl::begin()
{
    pinMode(topLimitPin, INPUT);
    pinMode(bottomLimitPin, INPUT);
    pinMode(motorUpPin, OUTPUT);
    pinMode(motorDownPin, OUTPUT);
    stop();
}

// Non-blocking state machine called every loop iteration.
// Flow: on startup, captures first valid sensor reading as reference.
// Then monitors ullage — if it shrinks by 5cm, lowers the platform
// until the reference distance is restored.
void PlatformControl::update()
{
    float distance = getSensorDistance();

    // in auto mode, handle reference setup and sensor-based compensation
    if (!manualMode)
    {
        // on first run, capture the current distance as the target reference
        if (!referenceSet)
        {
            if (distance > 0)
            {
                if (distance < MIN_REFERENCE_DISTANCE)
                {
                    // below minimum ullage — force reference to minimum and lower immediately
                    referenceDistance = MIN_REFERENCE_DISTANCE;
                    referenceSet = true;
                    float drop = MIN_REFERENCE_DISTANCE - distance;
                    Serial1.print("platform: ullage too low, going down by ");
                    Serial1.print(drop);
                    Serial1.println("cm");
                    moveDown();
                    state = MOVING_DOWN;
                }
                else
                {
                    referenceDistance = distance;
                    referenceSet = true;
                }
            }
            return;
        }
    }

    // safety: stop downward movement if bottom limit switch is hit
    if (isBottomLimit())
    {
        if (state == MOVING_DOWN)
        {
            Serial1.println("platform: bottom limit reached, stopping");
            stop();
        }
        else if (state == DEAD_TIME && pendingState == MOVING_DOWN)
        {
            Serial1.println("platform: bottom limit, cancelling pending move down");
            state = IDLE;
        }
    }

    // safety: stop upward movement if top limit switch is hit
    if (isTopLimit())
    {
        if (state == MOVING_UP)
        {
            Serial1.println("platform: top limit reached, stopping");
            stop();
        }
        else if (state == DEAD_TIME && pendingState == MOVING_UP)
        {
            Serial1.println("platform: top limit, cancelling pending move up");
            state = IDLE;
        }
    }

    switch (state)
    {
    case IDLE:
        // auto-compensation: ullage dropped below threshold, start lowering
        if (!manualMode && distance > 0 && distance <= (referenceDistance - COMPENSATION_THRESHOLD))
        {
            float drop = referenceDistance - distance;
            Serial1.print("platform: going down by ");
            Serial1.print(drop);
            Serial1.println("cm");
            moveDown();
        }
        break;

    case MOVING_DOWN:
        // auto-compensation: platform has lowered enough — ullage restored, stop motor
        if (!manualMode && distance >= referenceDistance)
        {
            Serial1.println("platform: distance restored, stopping");
            stop();
        }
        break;

    case MOVING_UP:
        // manual move — no auto-stop condition, user sends platform-stop
        break;

    case DEAD_TIME:
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
    referenceSet = true;
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

// drive platform up — enforces dead time if reversing from down
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
    if (state == DEAD_TIME) return;
    digitalWrite(motorUpPin, HIGH);
    digitalWrite(motorDownPin, LOW);
    state = MOVING_UP;
}

// drive platform down — enforces dead time if reversing from up
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

// cut power to both motor directions and cancel any pending dead time
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
