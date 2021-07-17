/* PINK - pink noise instrument

   Writes pink noise into the output buffer, with optional panning.

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = pan (in percent-to-left format) [optional, default is .5]

   p2 (amp) and p3 (pan) can receive updates from a table or real-time
   control source.

   Neil Thornock <neilthornock at gmail>, 11/12/16.

   Algorithm by Andrew Simper (vellocet.com/dsp/noise/VRand.html), who
   credits these people, mainly from the music-dsp mailing list:
   Allan Herriman, James McCartney, Phil Burk and Paul Kellet, and the
   web page by Robin Whittle: http://www.firstpr.com.au/dsp/pink-noise.
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "PINK.h"
#include <rt.h>
#include <rtdefs.h>

PINK::PINK()
	: _branch(0), _b0(0.0), _b1(0.0), _b2(0.0), _b3(0.0), _b4(0.0), _b5(0.0), _b6(0.0)
{
}

PINK::~PINK()
{
}

int PINK::init(double p[], int n_args)
{
	_nargs = n_args;		// store this for use in doupdate()

	const float outskip = p[0];
	const float dur = p[1];
	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("PINK", "Use mono or stereo output only.");

	return nSamps();
}

int PINK::configure()
{
	return 0;
}

void PINK::doupdate()
{
	double p[4];
	update(p, 4);

	_amp = p[2];

	_pan = (_nargs > 3) ? p[3] : 0.5;
}

int PINK::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		/*
			This is an approximation to a -10dB/decade filter using a weighted sum 
			of first order filters. It is accurate to within +/-0.05dB above 9.2Hz 
			(44100Hz sampling rate). Unity gain is at Nyquist, but can be adjusted 
			by scaling the numbers at the end of each line.  -Paul Kellet
		*/
		const double white = rrand() * _amp;
		_b0 = _b0 * 0.99886 + white * 0.0555179;
		_b1 = _b1 * 0.99332 + white * 0.0750759;
		_b2 = _b2 * 0.96900 + white * 0.1538520;
		_b3 = _b3 * 0.86650 + white * 0.3104856;
		_b4 = _b4 * 0.55000 + white * 0.5329522;
		_b5 = _b5 * -0.7616 - white * 0.0168980;
		double pink = _b0 + _b1 + _b2 + _b3 + _b4 + _b5 + _b6 + white * 0.5362;
		_b6 = white * 0.115926;

		float out[2];
		out[0] = pink * 0.15;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - _pan);
			out[0] *= _pan;
		}
		rtaddout(out);
		increment();
	}
	return framesToRun();
}

Instrument *makePINK()
{
	PINK *inst = new PINK();
	inst->set_bus_config("PINK");

	return inst;
}

#ifndef EMBEDDED
void rtprofile()
{
	RT_INTRO("PINK", makePINK);
}
#endif

