/* SPLITTER - split a mono input signal into any number of outputs

	The MIX instrument lets you route any of its input channels to a
   single destination.  SPLITTER is sort of the opposite: it lets you
   route a single input channel to multiple destinations.

   p0 = output start time
   p1 = input start time
   p2 = duration
   p3 = amplitude multiplier
   p4 = input channel [optional, default is 0]

   p3 (amp) can receive updates from a table or real-time control source.

   John Gibson <johgibso at indiana dot edu>, 1/22/06
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "SPLITTER.h"
#include <rt.h>
#include <rtdefs.h>


SPLITTER::SPLITTER()
	: _branch(0), _in(NULL)
{
}


SPLITTER::~SPLITTER()
{
	delete [] _in;
}


int SPLITTER::init(double p[], int n_args)
{
	const float outskip = p[0];
	const float inskip = p[1];
	const float dur = p[2];

	_inchan = (n_args > 4) ? int(p[4]) : 0;			// default is chan 0

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	if (_inchan >= inputChannels())
		return die("SPLITTER", "You asked for channel %d of a %d-channel input.",
		                                             _inchan, inputChannels());

	return nSamps();
}


int SPLITTER::configure()
{
	_in = new float [RTBUFSAMPS * inputChannels()];

	return _in ? 0 : -1;
}


void SPLITTER::doupdate()
{
	double p[4];
	update(p, 4, 1 << 3);

	_amp = p[3];
}


int SPLITTER::run()
{
	const int inchans = inputChannels();
	const int outchans = outputChannels();
	const int samps = framesToRun() * inchans;

	rtgetin(_in, this, samps);

	for (int i = 0; i < samps; i += inchans) {
		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float insig = _in[i + _inchan] * _amp;

		float out[outchans];

		for (int n = 0; n < outchans; n++)
			out[n] = insig;

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeSPLITTER()
{
	SPLITTER *inst = new SPLITTER();
	inst->set_bus_config("SPLITTER");

	return inst;
}


void rtprofile()
{
	RT_INTRO("SPLITTER", makeSPLITTER);
}


