/* STRUMFB - electric guitar model, based on original cmix strum

   Parameters marked with '*' can receive dynamic updates from a table or
   real-time control source.

     p0  = output start time
     p1  = duration
   * p2  = amplitude
   * p3  = frequency (Hz or oct.pc)
   * p4  = feedback frequency (Hz or oct.pc)
     p5  = squish
   * p6  = fundamental decay time
   * p7  = Nyquist decay time
   * p8  = distortion gain
   * p9  = feedback gain
   * p10 = clean signal level
   * p11 = distortion signal level
   * p12 = pan (in percent-to-left format) [optional, default is .5]

   John Gibson <johgibso at indiana dot edu>, 7/10/05
*/
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <Ougens.h>
#include "STRUMFB.h"
#include <rt.h>
#include <rtdefs.h>

const float kMinDecay = 0.001f;	// prevent NaNs in Ostrum

STRUMFB::STRUMFB()
	: _branch(0), _last(0), _delay(NULL), _distort(NULL), _strum(NULL)
{
}

STRUMFB::~STRUMFB()
{
	delete _delay;
	delete _distort;
	delete _strum;
}

int STRUMFB::init(double p[], int n_args)
{
	_nargs = n_args;
	const float outskip = p[0];
	const float dur = p[1];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("STRUMFB", "Use mono or stereo output only.");

	_rawfreq = p[3];
	float freq = (_rawfreq < 15.0) ? cpspch(_rawfreq) : _rawfreq;

	_rawfbfreq = p[4];
	float fbfreq = (_rawfbfreq < 15.0) ? cpspch(_rawfbfreq) : _rawfbfreq;

	_delsamps = 1.0 / fbfreq * SR;
	_delay = new Odelayi(long(_delsamps + 1.5));

	int squish = int(p[5]);

	_decaytime = p[6];
	if (_decaytime < kMinDecay)
		_decaytime = kMinDecay;
	_nyqdecaytime = p[7];
	if (_nyqdecaytime < kMinDecay)
		_nyqdecaytime = kMinDecay;

	_strum = new Ostrum(SR, freq, squish, _decaytime, _nyqdecaytime);

	_distort = new Odistort(Odistort::SoftClip);

	return nSamps();
}

int STRUMFB::configure()
{
	return 0;
}

void STRUMFB::doupdate()
{
	double p[13];
	update(p, 13, 1 << 2 | 1 << 3 | 1 << 4 | 1 << 6 | 1 << 7 | 1 << 8 | 1 << 9
					| 1 << 10 | 1 << 11 | 1 << 12);

	_amp = p[2];

	bool updateDecays = false;
	if (p[6] != _decaytime) {
		_decaytime = p[6];
		if (_decaytime < kMinDecay)
			_decaytime = kMinDecay;
		updateDecays = true;
	}
	if (p[7] != _nyqdecaytime) {
		_nyqdecaytime = p[7];
		if (_nyqdecaytime < kMinDecay)
			_nyqdecaytime = kMinDecay;
		updateDecays = true;
	}

	if (updateDecays) {
		_rawfreq = p[3];
		float freq = (_rawfreq < 15.0f) ? cpspch(_rawfreq) : _rawfreq;
		_strum->setfreqdecay(freq, _decaytime, _nyqdecaytime);
	}
	else if (p[3] != _rawfreq) {
		_rawfreq = p[3];
		float freq = (_rawfreq < 15.0f) ? cpspch(_rawfreq) : _rawfreq;
		_strum->setfreq(freq);
	}

	if (p[4] != _rawfbfreq) {
		_rawfbfreq = p[4];
		float fbfreq = (_rawfbfreq < 15.0f) ? cpspch(_rawfbfreq) : _rawfbfreq;
		_delsamps = 1.0 / fbfreq * SR;
	}

	_distgain = p[8];
	_fbgain = p[9] / _distgain;
	_cleanlevel = p[10];
	_distlevel = p[11];

	_pan = (_nargs > 12) ? p[12] : 0.5f;          // default is .5
}

int STRUMFB::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float cleansig = _strum->next(_last);
		float distsig = _distort->next(_distgain * cleansig);
		_delay->putsamp(distsig);
		_last = _fbgain * _delay->getsamp(_delsamps);

		float out[2];
		out[0] = (_cleanlevel * cleansig) + (_distlevel * distsig);
		out[0] *= _amp;

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0f - _pan);
			out[0] *= _pan;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}

Instrument *makeSTRUMFB()
{
	STRUMFB *inst = new STRUMFB();
	inst->set_bus_config("STRUMFB");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("STRUMFB", makeSTRUMFB);
}
#endif

