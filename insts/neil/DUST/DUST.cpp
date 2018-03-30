/* DUST - inspired by the Dust and Dust2 ugens from SuperCollider

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = density (average impulses per second) [default: 5]
   p4 = impulse range minimum (-1 or 0) [default: -1]
   p5 = seed [default: system clock]
   p6 = pan (in percent-to-left format) [default: 0.5]

   p2 (amp), p4 (density), and p6 (pan) can receive updates from a table or
   real-time control source.

   Neil Thornock, 11/2016; rev. 1/2018
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include "DUST.h"
#include <rt.h>
#include <rtdefs.h>
#include <random>
#include <ctime>

DUST::DUST()
	: _branch(0)
{
}

DUST::~DUST()
{
}

int DUST::init(double p[], int n_args)
{
	_nargs = n_args;

	const float outskip = p[0];
	const float dur = p[1];
	_dice = new Orand();
	_dice->timeseed();
	_range = (_nargs > 4) ? p[4] : -1;
	if (!((_range == 0) || (_range == -1)))
		return die("DUST", "p[4] range minimum must be either -1 or 0.");

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("DUST", "Use mono or stereo output only.");

	float seed = (_nargs > 5) ? p[5] : std::time(0);

	_randgen = std::mt19937(seed);
	_dist = std::uniform_real_distribution<double>(0, SR);

	return nSamps();
}

int DUST::configure()
{
	return 0;
}

void DUST::doupdate()
{
	double p[_nargs];
	update(p, _nargs);

	_amp = p[2];
	_density = (_nargs > 3) ? p[3] : 5;
	_pan = (_nargs > 6) ? p[6] : 0.5;
}

int DUST::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float out[2];

		float outsamp = 0.0;
		double thisr = _dist(_randgen);
		if (thisr < _density) {
			if (_range == -1)
				outsamp = _dice->rand() * _amp;
			else
				outsamp = _dice->random() * _amp;
		}

		out[0] = outsamp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - _pan);
			out[0] *= _pan;
		}
		rtaddout(out);
		increment();
	}

	return framesToRun();
}

Instrument *makeDUST()
{
	DUST *inst = new DUST();
	inst->set_bus_config("DUST");

	return inst;
}

#ifndef EMBEDDED
void rtprofile()
{
	RT_INTRO("DUST", makeDUST);
}
#endif
