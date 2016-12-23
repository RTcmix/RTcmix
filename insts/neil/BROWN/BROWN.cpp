/* BROWN - brown noise instrument

   Writes brown noise into the output buffer, with optional panning.

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
#include "BROWN.h"
#include <rt.h>
#include <rtdefs.h>

BROWN::BROWN()
	: _branch(0), _brown(0.0)
{
}

BROWN::~BROWN()
{
}

int BROWN::init(double p[], int n_args)
{
	_nargs = n_args;

	const float outskip = p[0];
	const float dur = p[1];
	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() > 2)
		return die("BROWN", "Use mono or stereo output only.");
	return nSamps();
}

int BROWN::configure()
{
	return 0;
}

void BROWN::doupdate()
{
	double p[4];
	update(p, 4);
	_amp = p[2];
	_pan = (_nargs > 3) ? p[3] : 0.5;
}

int BROWN::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}
		float out[2];

		while (true) {
			float r = rrand();
			_brown += r;
			if (_brown <- 8.0 || _brown > 8.0)
				_brown -= r;
			else
				break;
		}

		out[0] = _brown * 0.125 * _amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - _pan);
			out[0] *= _pan;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}

Instrument *makeBROWN()
{
	BROWN *inst = new BROWN();
	inst->set_bus_config("BROWN");

	return inst;
}

void rtprofile()
{
	RT_INTRO("BROWN", makeBROWN);
}

