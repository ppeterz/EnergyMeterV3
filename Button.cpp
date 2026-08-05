#include "Button.h"

void Button::begin(int p)
{
    pin = p;
    pinMode(pin, INPUT_PULLUP);
}

bool Button::pressed()
{
    bool reading = digitalRead(pin);
    bool result = false;

    if (reading != lastReading) lastDebounceMs = millis();

    if ((millis() - lastDebounceMs) > DEBOUNCE_MS && reading != state)
    {
        state = reading;
        if (state == LOW) {
            result = true;   // INPUT_PULLUP: LOW = pressed
            pressStartMs = millis();
            lastTriggeredMs = 0;
        }
    }

    lastReading = reading;
    return result;
}

bool Button::heldFor(unsigned long minMs, unsigned long maxMs)
{
    bool reading = digitalRead(pin);
    if (reading == LOW && pressStartMs > 0)
    {
        unsigned long elapsed = millis() - pressStartMs;
        if (elapsed >= minMs && elapsed < maxMs && lastTriggeredMs < pressStartMs + minMs)
        {
            lastTriggeredMs = millis();
            return true;
        }
    }
    return false;
}

bool Button::held(unsigned long targetMs)
{
    bool reading = digitalRead(pin);
    if (reading == LOW && pressStartMs > 0)
    {
        unsigned long elapsed = millis() - pressStartMs;
        if (elapsed >= targetMs && lastTriggeredMs < pressStartMs + targetMs)
        {
            lastTriggeredMs = millis();
            return true;
        }
    }
    return false;
}
