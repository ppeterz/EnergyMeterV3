#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button
{
public:
    void begin(int pin);
    bool pressed();   // true exactly once per press, debounced

private:
    int pin;
    bool lastReading = HIGH;
    bool state = HIGH;
    unsigned long lastDebounceMs = 0;
    static const unsigned long DEBOUNCE_MS = 20;
};

#endif
