#include "platform_control.h"

PlatformControl::PlatformControl(uint8_t topPin, uint8_t bottomPin,
                                 uint8_t upPin, uint8_t downPin,
                                 HcSr05 *hc)
    : topLimitPin(topPin), bottomLimitPin(bottomPin),
      motorUpPin(upPin), motorDownPin(downPin),
      hcSensor(hc),
      state(IDLE), pendingState(IDLE), deadTimeStart(0), compensationTarget(0) {}

float PlatformControl::getSensorDistance()
{
    return hcSensor ? hcSensor->getDistance() : -1.0;
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
// Monitors ullage — if it shrinks by 5cm from referenceDistance, lowers the
// platform until the reference distance is restored.
void PlatformControl::update()
{
    float distance = getSensorDistance();

    // safety: stop only when moving INTO the active limit switch
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
        Serial1.println("platform: limit switch, cancelling pending move");
        state = IDLE;
    }

    switch (state)
    {
    case IDLE:
        // trigger compensation whenever ullage drops below reference
        if (!manualMode && distance > 0 && distance <= referenceDistance)
        {
            compensationTarget = distance + COMPENSATION_STEP;
            Serial1.print("platform: lowering 5cm (ullage ");
            Serial1.print(distance);
            Serial1.println("cm)");
            moveDown();
        }
        break;

    case MOVING_DOWN:
        // stop when platform has lowered by 5cm (ullage increased by 5cm)
        if (!manualMode && distance >= compensationTarget)
        {
            Serial1.println("platform: compensation done, stopping");
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
