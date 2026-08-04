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
        if (state == LOW) result = true;   // INPUT_PULLUP: LOW = pressed
    }

    lastReading = reading;
    return result;
}
