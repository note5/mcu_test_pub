#ifndef HCSRO4_CTRL_H
#define HCSRO4_CTRL_H

#include "common.h"

// Pin assignments for HC-SR04 ultrasonic sensor
const int trigPin = PA0; // Trigger pin (GPIO output)
const int echoPin = PA1; // Echo pin (TIM2_CH2 input capture)

namespace Hcsr04
{
    // ============================================================================
    // TIMER-BASED HC-SR04 DRIVER
    // ============================================================================
    // Uses STM32 hardware timer (TIM2) for precise, interrupt-driven pulse measurement
    // - PA0: Trigger output (sends 10us pulse to start measurement)
    // - PA1: Echo input (TIM2_CH2 captures pulse width via interrupts)
    // - Timer runs at 1MHz (1 microsecond resolution)
    // - Non-blocking ISR captures rising/falling edges automatically
    // ============================================================================

    // State machine variables (modified by ISR, must be volatile)
    volatile uint32_t echo_start = 0;           // Timer count at echo rising edge
    volatile uint32_t echo_end = 0;             // Timer count at echo falling edge
    volatile bool measurement_complete = false; // Flag: measurement done
    volatile bool echo_active = false;          // Flag: waiting for falling edge

    // Measurement results (accessible to main code)
    long duration;     // Pulse duration in microseconds
    float distance_cm; // Calculated distance in centimeters

    // Hardware timer instance
    HardwareTimer *timer2 = nullptr;

    // ============================================================================
    // INTERRUPT SERVICE ROUTINE (ISR)
    // ============================================================================
    // Called automatically by hardware when PA1 changes state (rising/falling edge)
    // Implements a 2-state machine:
    //   1. Rising edge:  Record start time, set echo_active
    //   2. Falling edge: Record end time, calculate distance, set complete flag
    // ============================================================================
    void echoInterruptHandler()
    {
        if (timer2 == nullptr)
            return;

        // Read the captured timer value (exact microsecond count when edge occurred)
        uint32_t current_count = timer2->getCaptureCompare(2);

        if (!echo_active)
        {
            // STATE 1: Rising edge detected - echo pulse started
            echo_start = current_count;
            echo_active = true;
            measurement_complete = false;
        }
        else
        {
            // STATE 2: Falling edge detected - echo pulse ended
            echo_end = current_count;
            echo_active = false;
            measurement_complete = true;

            // Calculate pulse duration in microseconds
            if (echo_end >= echo_start)
            {
                // Normal case: no timer overflow
                duration = echo_end - echo_start;
            }
            else
            {
                // Timer overflow case: handle 32-bit wraparound
                duration = (0xFFFFFFFF - echo_start) + echo_end + 1;
            }

            // Convert duration to distance
            // Speed of sound: 343 m/s = 0.0343 cm/us
            // Divide by 2 because sound travels to object and back (round trip)
            distance_cm = duration * 0.0343 / 2.0;
        }
    }

    // ============================================================================
    // INITIALIZATION
    // ============================================================================
    // Sets up PA0 as trigger output and PA1 as timer input capture
    // Configures TIM2 for 1MHz operation (1us resolution) with both-edge capture
    // ============================================================================
    void init()
    {
        // Configure PA0 as digital output for trigger pulse
        pinMode(trigPin, OUTPUT);
        digitalWrite(trigPin, LOW);

        // Initialize TIM2 (32-bit timer) for input capture
        TIM_TypeDef *Instance = TIM2;
        timer2 = new HardwareTimer(Instance);

        // Configure timer prescaler for 1MHz (1 microsecond per tick)
        // Example: If system clock is 72MHz, prescaler will be 72
        timer2->setPrescaleFactor(timer2->getTimerClkFreq() / 1000000);

        // Set maximum overflow value for 32-bit timer
        // This gives ~71 minutes before overflow (more than enough for HC-SR04)
        timer2->setOverflow(0xFFFFFFFF);

        // Configure Channel 2 (PA1) for input capture on both rising and falling edges
        // This allows us to capture both the start and end of the echo pulse
        timer2->setMode(2, TIMER_INPUT_CAPTURE_BOTHEDGE, echoPin);

        // Attach our ISR to be called on each edge detection
        timer2->attachInterrupt(2, echoInterruptHandler);

        // Start the timer counting
        timer2->resume();

        debugln("HC-SR04: Timer-based driver initialized (PA0=Trig, PA1=Echo/TIM2_CH2)");
    }

    // Internal state tracking for non-blocking operation
    uint32_t last_trigger_time = 0;
    uint32_t measurement_start_time = 0;
    bool measurement_in_progress = false;
    // ============================================================================
    // NON-BLOCKING MEASUREMENT
    // ============================================================================
    // Call this repeatedly in your loop - it handles timing automatically
    // - Enforces 60ms minimum between measurements (HC-SR04 requirement)
    // - Automatically triggers new measurements when ready
    // - Handles timeout detection (100ms max wait)
    // - Updates distance_cm when measurement completes
    // - Returns true when new data is available
    // ============================================================================
    bool measure()
    {
        uint32_t now = millis();
        // State 1: Ready to start new measurement
        if (!measurement_in_progress)
        {
            // Check if enough time has passed since last trigger (60ms minimum)
            if (now - last_trigger_time >= 60)
            {
                // Safety check: if state machine is stuck, force reset
                if (echo_active)
                {
                    echo_active = false;
                    delayMicroseconds(100);
                }
                // Clear completion flag
                measurement_complete = false;

                // Send 10us trigger pulse to HC-SR04
                digitalWrite(trigPin, LOW);
                delayMicroseconds(2);
                digitalWrite(trigPin, HIGH);
                delayMicroseconds(10);
                digitalWrite(trigPin, LOW);

                // Update state
                last_trigger_time = now;
                measurement_start_time = now;
                measurement_in_progress = true;
            }
            return false; // No new data yet
        }

        // State 2: Measurement in progress - check for completion or timeout
        if (measurement_complete)
        {
            // Measurement successful!
            measurement_in_progress = false;
            return true; // New data available
        }
        else if (now - measurement_start_time > 100)
        {
            // Timeout occurred (100ms max, HC-SR04 max range ~4m = ~23ms)
            echo_active = false;
            distance_cm = -1.0;
            measurement_in_progress = false;
            debugln("HC-SR04: Measurement timeout");
            return true; // Return true to indicate state change (error)
        }

        return false; // Still waiting for measurement
    }
    //
    float getDistance()
    {
        return distance_cm;
    }
}

#endif