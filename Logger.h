#ifndef LOGGER_H
#define LOGGER_H

#include "Measurement.h"
#include "Energy.h"
#include "Config.h"

class Logger
{
public:
    void print(const Measurement m[NUM_CHANNELS], Energy energy[NUM_CHANNELS]);
};

#endif

