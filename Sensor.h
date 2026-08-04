#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include "Config.h"

class Sensor
{
public:
    void begin();
    void capture();

    uint16_t voltageBuffer[BUFFER_SIZE];
    uint16_t currentBuffer[NUM_CHANNELS][BUFFER_SIZE];   // 0=input, 1..NUM_SOCKETS=sockets
};

#endif
