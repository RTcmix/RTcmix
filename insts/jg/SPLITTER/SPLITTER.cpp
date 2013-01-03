/* SPLITTER - split a mono input signal into any number of outputs

	The MIX instrument lets you route any of its input channels to a
   single destination.  SPLITTER is sort of the opposite: it lets you
   route a single input channel to multiple destinations.

   p0 = output start time
   p1 = input start time
   p2 = duration
   p3 = global amp multiplier
   p4 = input channel
   p5 = amplitude multiplier for first output channel
   p6 = amplitude multiplier for second output channel
   etc. (number of amp mult args must match bus_config'd output chans)

   The amp multipliers can receive updates from a table or real-time
   control source.

   John Gibson <johgibso at indiana dot edu>, 1/22/06
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "SPLITTER.h"
#include <rt.h>
#include <rtdefs.h>


SPLITTER::SPLITTER()
	: _branch(0), _in(NULL), _amps(NULL)
{
}


SPLITTER::~SPLITTER()
{
	delete [] _in;
	delete [] _amps;
}


int SPLITTER::init(double p[], int n_args)
{
	_nargs = n_args;
	const float outskip = p[0];
	const float inskip = p[1];
	const float dur = p[2];
	_inchan = int(p[4]);

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	if (_inchan >= inputChannels())
		return die("SPLITTER", "You asked for channel %d of a %d-channel input.",
		                                             _inchan, inputChannels());

	if (_nargs - 5 != outputChannels())
		return die("SPLITTER", "You supplied amp multipliers for %d output "
		           "channels, but the instrument is configured for %d channels.",
		           _nargs - 5, outputChannels());

	_amps = new float [outputChannels()];

	return nSamps();
}


int SPLITTER::configure()
{
	_in = new float [RTBUFSAMPS * inputChannels()];

	return _in ? 0 : -1;
}


void SPLITTER::doupdate()
{
	double p[_nargs];
	update(p, _nargs);

	_amp = p[3];

	const int outchans = outputChannels();
	for (int i = 0; i < outchans; i++)
		_amps[i] = p[i + 5];
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
			out[n] = insig * _amps[n];

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

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("SPLITTER", makeSPLITTER);
}
#endif

