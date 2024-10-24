//
// Created by Douglas Scott on 10/22/24.
//

#include "PVBandThreshold.h"

#include <ugens.h>
#include <string.h>
#include <stdio.h>

PVFilter *
PVBandThreshold::create()
{
    return new PVBandThreshold;
}

PVBandThreshold::PVBandThreshold() : _frameDuration(0.0f), _binGains(NULL)
{
}

PVBandThreshold::~PVBandThreshold()
{
    delete [] _binGains;
}

void
PVBandThreshold::setFrameDuration(double duration)
{
    _frameDuration = duration;
    _attackIncrement = _frameDuration / _attackTime;
    _decayIncrement = _frameDuration / _decayTime;
//    printf("setFrameDuration(%f): attack incr %f decay incr %f\n", duration, _attackIncrement, _decayIncrement);
}

int
PVBandThreshold::run(float *pvdata, int nvals)
{
 //   printf("PVBandThreshold::run()\n");
    for (int n = 0; n < nvals; ++n) {
        const int ampIdx = n * 2;
        float binAmp = pvdata[ampIdx];
        if (binAmp > _ampThreshold) {
            float gain = _binGains[n];
            if (gain < 1.0f) {
//                if (n % 7) printf("gain %d (%f) += increment (%f)\n", ampIdx, gain, _attackIncrement);
                gain += _attackIncrement;
                gain = std::fmin(gain, 1.0f);
            }
            _binGains[n] = gain;
        }
        else {
            float gain = _binGains[n];
            if (gain > 0.0f) {
//                if (n % 7) printf("gain %d (%f) -= increment (%f)\n", ampIdx, gain, _decayIncrement);
                gain -= _decayIncrement;
                gain = std::fmax(gain, 0.0f);
            }
            _binGains[n] = gain;
        }
        pvdata[ampIdx] *= _binGains[n];
    }
    return 0;
}

// p0 = num bins, p1 = threshold, p2 = attackTime, p3 = decayTime

int
PVBandThreshold::init(double *pp, int nargs) {
    int pvocBins = (int) pp[0];
    _ampThreshold = pp[1];
    _attackTime = pp[2];
    _decayTime = pp[3];
    printf("init(): thresh %f attack time %f decay time %f\n", _ampThreshold, _attackTime, _decayTime);
    _binGains = new float[pvocBins];
    memset(_binGains, 0, pvocBins * sizeof(float));
    return 0;
}

// This function is called by the PVOC setup() routine for each DSO

extern "C" {
int registerLib(int (*register_fun)(FilterCreateFunction)) {
    return register_fun(&PVBandThreshold::create);
}
}
