#ifndef MEASUREMENT_H
#define MEASUREMENT_H

struct Measurement
{
    float voltage;

    float current;

    float realPower;

    float apparentPower;

    float powerFactor;

    float vrmsADC;

    float irmsADC;
};

#endif