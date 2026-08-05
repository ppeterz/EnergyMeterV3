#include "MeasurementEngine.h"
#include "Config.h"
#include "Calibration.h"
#include <math.h>

void MeasurementEngine::calculate(Sensor &sensor, Measurement out[NUM_CHANNELS])
{
    float voltScale    = calibration.getVoltScale();
    float currentScale = calibration.getCurrentScale();

    float voltageSum = 0, vrmsADCSum = 0;
    float currentSum[NUM_CHANNELS]  = {0};
    float realSum[NUM_CHANNELS]     = {0};
    float apparentSum[NUM_CHANNELS] = {0};
    float irmsADCSum[NUM_CHANNELS]  = {0};

    for (int n = 0; n < AVG_COUNT; n++)
    {
        sensor.capture();

        // Voltage is read once and shared across all channels -- matches
        // the physical wiring: one ZMPT101B tap feeds every socket's power
        // calculation, only the current differs per channel.
        float vOffset = dsp.mean(sensor.voltageBuffer);
        float vrmsADC = dsp.rms(sensor.voltageBuffer, vOffset);
        float voltage = vrmsADC * voltScale;
        voltageSum += voltage;
        vrmsADCSum += vrmsADC;

        for (int c = 0; c < NUM_CHANNELS; c++)
        {
            float iOffset = dsp.mean(sensor.currentBuffer[c]);
            float irmsADC = dsp.rms(sensor.currentBuffer[c], iOffset);
            float current = irmsADC * currentScale;

            float real = power.realPower(sensor.voltageBuffer, sensor.currentBuffer[c],
                                          vOffset, iOffset);
            float apparent = voltage * current;

            currentSum[c]  += current;
            realSum[c]     += real;
            apparentSum[c] += apparent;
            irmsADCSum[c]  += irmsADC;
        }
    }

    float avgVoltage = voltageSum / AVG_COUNT;
    float avgVrmsADC = vrmsADCSum / AVG_COUNT;

    for (int c = 0; c < NUM_CHANNELS; c++)
    {
        Measurement &m = out[c];
        m.voltage       = avgVoltage;
        m.current       = currentSum[c] / AVG_COUNT;
        m.realPower     = realSum[c] / AVG_COUNT;
        m.apparentPower = apparentSum[c] / AVG_COUNT;
        m.vrmsADC       = avgVrmsADC;
        m.irmsADC       = irmsADCSum[c] / AVG_COUNT;

        if (fabs(m.current) < CURRENT_DEADBAND || fabs(m.realPower) < POWER_DEADBAND)
        {
            m.current       = 0.0f;
            m.realPower     = 0.0f;
            m.apparentPower = 0.0f;
            m.powerFactor   = 0.0f;
        }
        else
        {
            m.powerFactor = (m.apparentPower > 0.01f) ? (m.realPower / m.apparentPower) : 0.0f;
            if (m.powerFactor > 1.0f)  m.powerFactor = 1.0f;
            if (m.powerFactor < -1.0f) m.powerFactor = -1.0f;
        }
    }
}
