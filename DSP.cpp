#include "DSP.h"
#include <math.h>
#include "Config.h"

float DSP::mean(uint16_t *buffer)
{
    uint32_t sum=0;

    for(int i=0;i<BUFFER_SIZE;i++)
        sum+=buffer[i];

    return (float)sum/BUFFER_SIZE;
}

float DSP::rms(uint16_t *buffer,float offset)
{
    double sum=0;

    for(int i=0;i<BUFFER_SIZE;i++)
    {
        float x=buffer[i]-offset;

        sum+=x*x;
    }

    return sqrt(sum/BUFFER_SIZE);
}