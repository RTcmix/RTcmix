/* BWESINE - Bandwidth-enhanced sine wave oscillator, using the modulation
   technique employed by Loris (http://cerlsoundgroup.org/Loris) and described
   in this paper:
      "On the Use of Time-Frequency Reassignment in Additive Sound Modeling,"
      Kelly Fitz and Lippold Haken, Journal of the Audio Engineering Society,
      vol. 50, no. 11, November 2002.
   This allows us to resynthesize sound models written by Loris.

   NB: The phase pfield doesn't work yet.

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = freq (in Hz only)
   p4 = bandwidth
   p5 = starting phase (radians)
   p6 = pan (in percent-to-left format) [optional, default is .5]
   p7 = wavetable

   p2 (amp), p3 (freq), p4 (bandwidth), and p5 (pan) can receive updates from
   a table or real-time control source.

   You must supply a wavetable handle, so none of the pfields are optional.
   for example:

      wavetable = maketable("wave", 32767, "sine")

   John Gibson <johgibso at indiana dot edu>, 10/11/17.
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ugens.h>
#include <Ougens.h>
#include "BWESINE.h"
#include <rt.h>
#include <rtdefs.h>

const double seed = 1.0;
const double mean = 0.0;
const double stddev = 1.0;

BweNoise::BweNoise()
	: _rgen(seed),
	  _dist(mean, stddev)
{
	for (int i = 0; i < 4; i++)
		_xv[i] = _yv[i] = 0.0;
}

BweNoise::~BweNoise()
{
}

const double kGain = 4.663939207e+04;

float BweNoise::next()
{
	double x = _dist(_rgen);  // return value from normal (Gaussian) distribution

	// Chebyshev 3rd-order lowpass, cf: 500 Hz, ripple: -1 dB, srate: 44100
	// Specified at, and with code adapted from:
	// http://www-users.cs.york.ac.uk/~fisher/mkfilter
	_xv[0] = _xv[1];
	_xv[1] = _xv[2];
	_xv[2] = _xv[3];
	_xv[3] = x / kGain;
	_yv[0] = _yv[1];
	_yv[1] = _yv[2];
	_yv[2] = _yv[3];
	_yv[3] = (_xv[0] + _xv[3]) + (3 * (_xv[1] + _xv[2]))
					+ (0.9320209047 * _yv[0]) + (-2.8580608588 * _yv[1])
					+ (2.9258684253 * _yv[2]);
	return _yv[3];
}


BWESINE::BWESINE()
	: _noi(NULL), _osc(NULL), _branch(0)
{
}

BWESINE::~BWESINE()
{
	delete _noi;
	delete _osc;
}

#define USAGE "BWESINE(start, dur, amp, freq, bandwidth, phase, pan, wavetable)"

int BWESINE::init(double p[], int n_args)
{
	if (n_args != 8)
		return die("BWESINE", "Usage: " USAGE);

	const float outskip = p[0];
	const float dur = p[1];
	const float initPhase = p[5];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("BWESINE", "Use mono or stereo output only.");

	int tablelen = 0;
	double *wavetable = (double *) getPFieldTable(7, &tablelen);
	if (wavetable == NULL)
		return die("WAVETABLE", "No wavetable specified.");
	if (tablelen > 32767)
		return die("WAVETABLE", "wavetable must have fewer than 32768 samples.");

	_noi = new BweNoise();
	_osc = new Ooscili(SR, 1, wavetable, tablelen);

	return nSamps();
}

int BWESINE::configure()
{
	return 0;
}

const float kNormalizer = 1.0 / 32768.0;
const float kDeNormalizer = 32768.0;

void BWESINE::doupdate()
{
	double p[7];
	update(p, 7);
	_amp = p[2] * kNormalizer;
	_osc->setfreq(p[3]);
	_bandwidth = p[4];
	if (_bandwidth > 1.0)
		_bandwidth = 1.0;
	else if (_bandwidth < 0.0)
		_bandwidth = 0.0;
	_pan = p[6];
}

int BWESINE::run()
{
	for (int i = 0; i < framesToRun(); i++) {
		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float out[2];
		double amp = _amp;
		if (_bandwidth == 1.0) {
			amp *= sqrt(2.0 * _bandwidth) * _noi->next();
		}
		else if (_bandwidth != 0.0) {
			double a = sqrt(1.0 - _bandwidth);
			a += sqrt(2.0 * _bandwidth) * _noi->next();
			amp *= a;
		}
		out[0] = _osc->next() * amp * kDeNormalizer;
//printf("phase: %f\n", _osc->getphase());

		if (outputChannels() == 2) {
			out[1] = out[0] * (1.0 - _pan);
			out[0] *= _pan;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}

Instrument *makeBWESINE()
{
	BWESINE *inst = new BWESINE();
	inst->set_bus_config("BWESINE");
	return inst;
}

#ifndef EMBEDDED
void rtprofile()
{
	RT_INTRO("BWESINE", makeBWESINE);
}
#endif

