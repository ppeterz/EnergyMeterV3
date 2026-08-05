#include "Button.h"

void Button::begin(int p)
{
    pin = p;
    pinMode(pin, INPUT_PULLUP);
}

ButtonEvent Button::update()
{
    bool reading = digitalRead(pin);
    ButtonEvent event = BTN_NONE;

    // Debounce timing
    if (reading != lastReading)
    {
        lastDebounceMs = millis();
    }

    if ((millis() - lastDebounceMs) > DEBOUNCE_MS)
    {
        // State change detected
        if (reading != state)
        {
            state = reading;

            if (state == LOW)
            {
                // Button Pressed Down
                pressStartMs = millis();
                mediumTriggered = false;
                longTriggered = false;
            }
            else
            {
                // Button Released
                unsigned long duration = millis() - pressStartMs;
                // If it was a quick click and no hold action was triggered
                if (duration < 1500 && !mediumTriggered && !longTriggered)
                {
                    event = BTN_CLICK;
                }
                pressStartMs = 0;
            }
        }
    }

    lastReading = reading;

    // While button is continuously held down (LOW state)
    if (state == LOW && pressStartMs > 0)
    {
        unsigned long elapsed = millis() - pressStartMs;

        if (elapsed >= 4000 && !longTriggered)
        {
            longTriggered = true;
            event = BTN_HOLD_LONG;
        }
        else if (elapsed >= 1500 && elapsed < 4000 && !mediumTriggered)
        {
            mediumTriggered = true;
            event = BTN_HOLD_MEDIUM;
        }
    }

    return event;
}
