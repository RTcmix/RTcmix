#include <RTcmix.h>
#include <stdio.h>
#include <math.h>
#include <ugens.h>

extern "C" {
double tbase(double p[], int n_args);
double tempo(double p[], int n_args);
double time_beat(double timein);
double beat_time(double beatin);
};


double
tbase(double p[], int n_args)
{
    return RTcmix::tbase(p, n_args);
}

double
tempo(double p[], int n_args)
{
    return RTcmix::tempo(p, n_args);
}

double
time_beat(double timein)
{
    return RTcmix::time_beat(timein);
}

double
beat_time(double timein)
{
    return RTcmix::beat_time(timein);
}