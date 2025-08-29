#ifndef TIMING_H
#define TIMING_H

#include "common.h"

class SmartDelay
{
private:
    unsigned long previousMillis; // Stores when timer was last triggered
    unsigned long interval;       // How long to wait between triggers

public:
    // CONSTRUCTOR - runs when object is created
    SmartDelay(unsigned long _interval)
    {
        interval = _interval; // Set the timer interval
        previousMillis = 0;   // Initialize to 0 (will trigger immediately)
    }

    // METHOD 1: isReady() - Check if timer should trigger
    bool isReady()
    {
        unsigned long currentMillis = millis();

        // Calculate time elapsed since last trigger
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis; // Update last trigger time
            return true;                    // Timer is ready!
        }
        return false; // Still waiting...
    }

    // METHOD 2: setInterval() - Change the timer's delay period
    void setInterval(unsigned long newInterval)
    {
        interval = newInterval; // Update the interval member variable
    }

    // METHOD 3: reset() - Restart the timer from current moment
    void reset()
    {
        previousMillis = millis(); // Set "last trigger" to right now
    }

    // METHOD 4: getInterval() - Get current interval
    unsigned long getInterval() const
    {
        return interval;
    }

    // METHOD 5: getElapsed() - Get time elapsed since last trigger
    unsigned long getElapsed() const
    {
        return millis() - previousMillis;
    }

    // METHOD 6: getRemaining() - Get time remaining until next trigger
    unsigned long getRemaining() const
    {
        unsigned long elapsed = getElapsed();
        return (elapsed >= interval) ? 0 : (interval - elapsed);
    }
};

namespace Timing
{
    // Pre-defined timer to check orientation
    SmartDelay orientationUpdateTimer(500);    // 500ms for orientation updates

    
    // Convenience function to update orienation
    bool shouldUpdateOrientation() { return orientationUpdateTimer.isReady(); }
  
}

#endif