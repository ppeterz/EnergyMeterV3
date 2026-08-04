#ifndef COMMAND_H
#define COMMAND_H

#include "Relay.h"
#include "Energy.h"
#include "Sensor.h"
#include "DSP.h"
#include "Config.h"
#include "Tariff.h"

class Command
{
public:
    void process(Relay relay[NUM_SOCKETS], Energy energy[NUM_CHANNELS], Sensor &sensor);

private:
    DSP dsp;   // used only for the calibration capture
};

#endif

