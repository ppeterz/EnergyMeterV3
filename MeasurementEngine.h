#ifndef MEASUREMENT_ENGINE_H
#define MEASUREMENT_ENGINE_H

#include "Sensor.h"
#include "DSP.h"
#include "Power.h"
#include "Measurement.h"
#include "Config.h"

class MeasurementEngine
{
public:
    void calculate(Sensor &sensor, Measurement out[NUM_CHANNELS]);

private:
    DSP dsp;
    Power power;

    // Kept at 1 deliberately: each capture already averages 10 whole AC
    // cycles (800 samples / 80 per cycle) via RMS math, which is already
    // fairly stable. Repeating the whole capture AVG_COUNT times (as the
    // single-channel calibration rig did) costs real time budget we now
    // need for Blynk/LCD/button responsiveness. Raise this later if you
    // want smoother numbers and can afford the extra blocking time.
    static const int AVG_COUNT = 1;
};

#endif
