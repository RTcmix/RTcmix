/* STRUM2 - plucked string model, based on original cmix strum

   p0 = output start time
   p1 = duration
   p2 = amplitude
   p3 = frequency (Hz or oct.pc)
   p4 = squish
   p5 = decay time
   p6 = pan (in percent-to-left format) [optional, default is .5]

   p2 (amp), p3 (freq) and p6 (pan) can receive updates from a table or
   real-time control source.

   John Gibson <johgibso at indiana dot edu>, 7/10/05
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include "STRUM2.h"
#include <rt.h>
#include <rtdefs.h>

const float kMinDecay = 0.001f;	// prevent NaNs in Ostrum

STRUM2::STRUM2()
	: _branch(0), _strum(NULL)
{
}

STRUM2::~STRUM2()
{
	delete _strum;
}

int STRUM2::init(double p[], int n_args)
{
	_nargs = n_args;
	const float outskip = p[0];
	const float dur = p[1];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("STRUM2", "Use mono or stereo output only.");

	_rawfreq = p[3];
	float freq = (_rawfreq < 15.0) ? cpspch(_rawfreq) : _rawfreq;

	int squish = int(p[4]);

	float fundDecayTime = p[5];
	if (fundDecayTime < kMinDecay)
		fundDecayTime = kMinDecay;
	float nyquistDecayTime = fundDecayTime * 0.1;

	_strum = new Ostrum(SR, freq, squish, fundDecayTime, nyquistDecayTime);

	return nSamps();
}

int STRUM2::configure()
{
	return 0;
}

void STRUM2::doupdate()
{
	double p[7];
	update(p, 7, 1 << 2 | 1 << 3 | 1 << 6);

	_amp = p[2];

	if (p[3] != _rawfreq) {
		_rawfreq = p[3];
		float freq = (_rawfreq < 15.0) ? cpspch(_rawfreq) : _rawfreq;
		_strum->setfreq(freq);
	}

	_pan = (_nargs > 6) ? p[6] : 0.5;           // default is .5
}

int STRUM2::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float out[2];

		out[0] = _strum->next() * _amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0f - _pan);
			out[0] *= _pan;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}

Instrument *makeSTRUM2()
{
	STRUM2 *inst = new STRUM2();
	inst->set_bus_config("STRUM2");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("STRUM2", makeSTRUM2);
}
#endif

