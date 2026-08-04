#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button
{
public:
    void begin(int pin);
    bool pressed();              // short press click
    bool held(unsigned long ms); // held for at least N ms (triggers once per hold)

private:
    int pin;
    bool lastReading = HIGH;
    bool state = HIGH;
    unsigned long lastDebounceMs = 0;
    unsigned long pressStartMs = 0;
    bool holdTriggered = false;
    static const unsigned long DEBOUNCE_MS = 20;
};

#endif
