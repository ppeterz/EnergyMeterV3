#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button
{
public:
    void begin(int pin);
    bool pressed();                                          // short press click
    bool heldFor(unsigned long minMs, unsigned long maxMs);  // held between minMs and maxMs
    bool held(unsigned long ms);                             // held at least ms

private:
    int pin;
    bool lastReading = HIGH;
    bool state = HIGH;
    unsigned long lastDebounceMs = 0;
    unsigned long pressStartMs = 0;
    unsigned long lastTriggeredMs = 0;
    static const unsigned long DEBOUNCE_MS = 20;
};

#endif
