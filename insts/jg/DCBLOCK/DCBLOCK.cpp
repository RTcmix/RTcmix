/* DCBLOCK - remove (most of) DC bias from input signal

   p0 = output start time
   p1 = input start time
   p2 = duration
   p3 = amp multiplier

   The amp multiplier can receive updates from a table or real-time
   control source.

   DCBLOCK processes N input channels to N output channels, e.g. mono
   to mono, stereo to stereo, quad to quad, etc.

   John Gibson <johgibso at indiana dot edu>, 5/21/06
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include "DCBLOCK.h"
#include <rt.h>
#include <rtdefs.h>


DCBLOCK::DCBLOCK()
	: _branch(0), _chans(0), _in(NULL), _blocker(NULL)
{
}


DCBLOCK::~DCBLOCK()
{
	for (int i = 0; i < _chans; i++)
		delete _blocker[i];
	delete [] _blocker;
	delete [] _in;
}


int DCBLOCK::init(double p[], int n_args)
{
	const float outskip = p[0];
	const float inskip = p[1];
	const float dur = p[2];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() != inputChannels())
		return die("DCBLOCK", "The number of input channels must be the same "
		                      "as the number of output channels.");
	_chans = inputChannels();

	_blocker = new Odcblock * [_chans];
	for (int i = 0; i < _chans; i++)
		_blocker[i] = new Odcblock();

	return nSamps();
}


int DCBLOCK::configure()
{
	_in = new float [RTBUFSAMPS * inputChannels()];

	return _in ? 0 : -1;
}


void DCBLOCK::doupdate()
{
	double p[4];
	update(p, 4);

	_amp = p[3];
}


int DCBLOCK::run()
{
	const int samps = framesToRun() * _chans;

	rtgetin(_in, this, samps);

	for (int i = 0; i < samps; i += _chans) {
		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float out[_chans];
		for (int n = 0; n < _chans; n++)
			out[n] = _blocker[n]->next(_in[i + n]) * _amp;

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeDCBLOCK()
{
	DCBLOCK *inst = new DCBLOCK();
	inst->set_bus_config("DCBLOCK");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("DCBLOCK", makeDCBLOCK);
}
#endif

