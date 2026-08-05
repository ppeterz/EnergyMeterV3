#include "Power.h"
#include "Config.h"
#include "Calibration.h"

float Power::realPower(uint16_t *v,
                       uint16_t *i,
                       float voffset,
                       float ioffset,
                       int channel)
{
    double sum = 0;

    float voltScale    = calibration.getVoltScale();
    float currentScale = calibration.getCurrentScale(channel);

    for (int n = 0; n < BUFFER_SIZE; n++)
    {
        float voltage = (v[n] - voffset) * voltScale;
        float current = (i[n] - ioffset) * currentScale;

        sum += voltage * current;
    }

    return sum / BUFFER_SIZE;
}