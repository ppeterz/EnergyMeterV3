#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

enum ButtonEvent
{
    BTN_NONE = 0,
    BTN_CLICK,        // Quick press and release (< 1.5s) -> Cycle LCD Screen
    BTN_HOLD_MEDIUM,  // Held for 1.5s -> Toggle Socket Relay
    BTN_HOLD_LONG     // Held for 4.0s -> Firmware OTA Update
};

class Button
{
public:
    void begin(int pin);
    ButtonEvent update();  // Call once per loop() to get active event

private:
    int pin;
    bool lastReading = HIGH;
    bool state = HIGH;
    unsigned long lastDebounceMs = 0;
    unsigned long pressStartMs = 0;
    bool mediumTriggered = false;
    bool longTriggered = false;
    static const unsigned long DEBOUNCE_MS = 25;
};

#endif
