/* WAVY - dual wavetable oscillator instrument

      p0 = output start time
      p1 = duration
      p2 = amplitude (0-32767, as in WAVETABLE)
      p3 = frequency (Hz, or if < 15: oct.pc)
      p4 = phase offset for second oscillator (0-1)
      p5 = wavetable (use maketable("wave", ...) for this)
      p6 = combination mode ("nocombine", "add", "subtract", "multiply")
      p7 = pan (in percent-to-left form: 0-1)

   p2 (amplitude), p3 (freq), p4 (phase offset) and p7 (pan) can receive
   updates from a table or real-time control source.  You can also update
	the wavetable (p5) using modtable(wavetable, "draw", ...).

   John Gibson, 6/15/05
*/
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <ugens.h>
#include <Ougens.h>
#include <Instrument.h>
#include <PField.h>
#include "WAVY.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG


WAVY::WAVY() : Instrument()
{
	_branch = 0;
	_freqraw = -DBL_MAX;
	_phaseOffset = -DBL_MAX;
	_oscil1 = NULL;
	_oscil2 = NULL;
}


WAVY::~WAVY()
{
	delete _oscil1;
	delete _oscil2;
}


int WAVY::usage() const
{
	return die("WAVY",
	           "Usage: WAVY(start, dur, amp, freq, phase_offset, wavetable, "
	           "mode, pan");
}


int WAVY::getCombinationMode() const
{
	const PField &field = getPField(6);
	const char *str = field.stringValue(0.0);
	if (str[0] == 'n')
		return kNoCombineMode;
	else if (str[0] == 'a')
		return kAddMode;
	else if (str[0] == 's')
		return kSubtractMode;
	else if (str[0] == 'm')
		return kMultiplyMode;
	return -1;
}


int WAVY::init(double p[], int n_args)
{
	if (n_args < 8)
		return usage();

	_nargs = n_args;
	const float outskip = p[0];
	const float dur = p[1];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() < 1 || outputChannels() > 2)
		return die("WAVY", "Must have mono or stereo output only.");

	int wavelen;
	double *wavet = (double *) getPFieldTable(5, &wavelen);
	if (wavet == NULL)
		return die("WAVY", "p5 must be wavetable (use maketable)");

	_oscil1 = new Ooscil(SR, 440.0, wavet, wavelen);
	_oscil2 = new Ooscil(SR, 440.0, wavet, wavelen);

	const int mode = getCombinationMode();
	if (mode == kNoCombineMode)
		setCombineFunc(nocombine);
	else if (mode == kAddMode)
		setCombineFunc(add);
	else if (mode == kSubtractMode)
		setCombineFunc(subtract);
	else if (mode == kMultiplyMode)
		setCombineFunc(multiply);

	return nSamps();
}


void WAVY::doupdate()
{
	double p[_nargs];
	update(p, _nargs, 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5 | 1 << 7);

	_amp = p[2];
	if (p[3] != _freqraw) {
		_freqraw = p[3];
		float freq = (_freqraw < 15.0) ? cpspch(_freqraw) : _freqraw;
		_oscil1->setfreq(freq);
		_oscil2->setfreq(freq);
	}
	if (p[4] != _phaseOffset) {
		_phaseOffset = p[4];
		// Get current phase of oscil1; offset oscil2 phase from this by
		// _phaseOffset, in units relative to wavetable length.
		int len = _oscil2->getlength();		// both wavetables are same length
		double phase2 = _oscil1->getphase() + (_phaseOffset * len);
		while (phase2 >= double(len))
			phase2 -= double(len);
		_oscil2->setphase(phase2);
	}
	_pan = p[7];
}


int WAVY::run()
{
	const int frames = framesToRun();
	const int chans = outputChannels();

	for (int i = 0; i < frames; i++) {
		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}

		float sig1 = _oscil1->nexti();
		float sig2 = _oscil2->nexti();

		float out[chans];
		out[0] = (*_combiner)(sig1, sig2) * _amp;

		if (chans == 2) {
			out[1] = out[0] * (1.0f - _pan);
			out[0] *= _pan;
		}

		rtaddout(out);
		increment();
	}

	return framesToRun();
}


Instrument *makeWAVEY()
{
	WAVY *inst = new WAVY();
	inst->set_bus_config("WAVY");

	return inst;
}


void rtprofile()
{
	RT_INTRO("WAVY", makeWAVY);
}

