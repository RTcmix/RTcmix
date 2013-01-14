/* PHASER

   Runs samples through a user-specified number of allpass filters.  It
   processes only one input channel for a given note.

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = stages (should be even number, recommended 2 to 16 or 24)
   p5 = LFO frequency (in cps)
   p6 = reverb time (seconds)
   p7 = wet/dry mix [optional (0.0 - 1.0), default is .5 (50% wet/50% dry)]
   p8 = input channel [optional, default is 0]
   p9 = pan (in percent-to-left format) [optional, default is .5]

   p3 (amp) and p9 (pan) can receive updates from a table or real-time
   control source.

   Jenny Bernard <bernarjf at email dot uc dot edu>, 12/7/05
*/

#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include <rt.h>
#include <rtdefs.h>
#include "PHASER.h"

using std::vector;

#define DEBUG 0

const float kRingDur = 0.1;


PHASER::PHASER()
	: _in(NULL), _branch(0)
{
}


PHASER::~PHASER()
{
	delete [] _in;
}


int PHASER::init(double p[], int n_args)
{
	_nargs = n_args;		// store this for use in doupdate()

	const float outskip = p[0];
	const float inskip = p[1];
	const float dur = p[2];
	_numfilters = int(p[4]);
	_lfofreq = p[5];
	_reverbtime = p[6];
	_wetdry = (n_args > 7) ? p[7] : .5;			// default is 0.5 (50% wet/50% dry)
	if (_wetdry < 0.0 || _wetdry > 1.0)
		_wetdry = 0.5;
	_inchan = (n_args > 8) ? int(p[8]) : 0;	// default is chan 0

	// no need to retrieve amp or pan here, because these will be set 
	// before their first use inside of doupdate().

	if (rtsetoutput(outskip, dur + kRingDur, this) == -1)
		return DONT_SCHEDULE;
	_insamps = int(dur * SR + 0.5);

	if (outputChannels() > 2)
		return die("PHASER", "Use mono or stereo output only.");

	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	if (_inchan >= inputChannels())
		return die("PHASER", "You asked for channel %d of a %d-channel input.",
		                                             _inchan, inputChannels());

	// the Oallpassi's
	float loopt = .01;
	
	lfo = new Ooscili(SR, _lfofreq, 2);

	_filtervector.resize(_numfilters);
#if DEBUG > 0
	printf("Vector successfully created.\n");
	printf("Vector size: %zd\n", _filtervector.size());
	printf("_reverbtime: %f\n", _reverbtime);
#endif

	for (int i = 0; i < _numfilters; i++) {
		allpassptr = new Oallpassi(SR, loopt, loopt+.5, _reverbtime);
		_filtervector[i] = allpassptr;
		if (_filtervector[i]->frequency() == 0.0)
			return die("PHASER", "Failed to allocate allpass memory!");
		loopt += 0.01;
	}

#if DEBUG > 0
	// This shows that the vector has been allocated with distinct filters
	for (int i = 0; i < _numfilters; i++) {
		printf("Oallpassi frequency in filtervector[%d]: %f\n", i,
		                                      _filtervector[i]->frequency());
	}
#endif

	return nSamps();
}


int PHASER::configure()
{
	_in = new float [RTBUFSAMPS * inputChannels()];
	return _in ? 0 : -1;
}


void PHASER::doupdate()
{
	double p[10];
	update(p, 10);

	_amp = p[3];

	_pan = (_nargs > 9) ? p[9] : 0.5;		// default is .5
}


int PHASER::run()
{
	const int samps = framesToRun() * inputChannels();

	if (currentFrame() < _insamps)
		rtgetin(_in, this, samps);

	for (int i = 0; i < samps; i += inputChannels()) {
		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		// Grab the current input sample, scaled by the amplitude multiplier.
		float insig = 0.0f;
		if (currentFrame() < _insamps)
			insig = _in[i + _inchan] * _amp;

		float delaysamps = lfo->next() * 0.5;

		// phasing loop
		float wetsig = insig;
		for (int j = 0; j < _numfilters; j++) {
//			float thisdelay = (SR / (_filtervector[j]->frequency()));
			wetsig = _filtervector[j]->next(wetsig, delaysamps);
		}

		// cut down volume of filter output before mixing wet and dry--this is a hack
//		wetsig = wetsig / float(_numfilters - (0.5 * (_numfilters - 1)); 

		// mix wet and dry
		float out[2];
		out[0] = (_wetdry * wetsig) + ((1.0f - _wetdry) * insig);

		// cut down volume after mixing wet and dry
//		out[0] = out[0] / (float(_numfilters) * .55); 

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0f - _pan);
			out[0] *= _pan;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makePHASER()
{
	PHASER *inst = new PHASER();
	inst->set_bus_config("PHASER");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("PHASER", makePHASER);
}
#endif
