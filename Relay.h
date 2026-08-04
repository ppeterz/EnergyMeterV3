#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

class Relay
{
public:
    void begin(int pin);
    void on();
    void off();
    void toggle();
    bool isOn();

private:
    int pin;
    bool state = false;
};

#endif
