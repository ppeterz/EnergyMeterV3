#ifndef POWER_H
#define POWER_H

#include <Arduino.h>

class Power
{
public:

    float realPower(uint16_t *v,
                    uint16_t *i,
                    float voffset,
                    float ioffset,
                    int channel = 0);

};

#endif