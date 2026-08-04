#ifndef DSP_H
#define DSP_H

#include <Arduino.h>

class DSP
{
public:

    float mean(uint16_t *buffer);

    float rms(uint16_t *buffer,float offset);

};

#endif