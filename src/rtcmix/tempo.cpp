#include <RTcmix.h>
#include <stdio.h>
#include <math.h>
#include <ugens.h>

extern "C" {
double tbase(double p[], int n_args);
double tempo(double p[], int n_args);
float time_beat(float timein);
float beat_time(float beatin);
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

float
time_beat(float timein)
{
    return RTcmix::time_beat(timein);
}

float
beat_time(float timein)
{
    return RTcmix::beat_time(timein);
}