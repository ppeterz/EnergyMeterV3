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
            holdTriggered = false;
        }
    }

    lastReading = reading;
    return result;
}

bool Button::held(unsigned long targetMs)
{
    bool reading = digitalRead(pin);
    if (reading == LOW && !holdTriggered)
    {
        if (pressStartMs > 0 && (millis() - pressStartMs >= targetMs))
        {
            holdTriggered = true;
            return true;
        }
    }
    if (reading == HIGH)
    {
        holdTriggered = false;
    }
    return false;
}
