// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

/* WAVY - dual wavetable oscillator instrument

      p0 = output start time
      p1 = duration
    * p2 = amplitude (0-32767, as in WAVETABLE)
    * p3 = oscil A frequency (Hz, or if < 15: oct.pc)
    * p4 = oscil B frequency (Hz, or if < 15: oct.pc; if zero, same as A)
    * p5 = phase offset for second oscillator (0-1)
      p6 = oscil A wavetable (use maketable("wave", ...) for this)
      p7 = oscil B wavetable (if zero, same as A)
      p8 = combination expression ("a + b", "a - b", "a * b", etc.; see note 2)
    * p9 = pan (in percent-to-left form: 0-1) [optional; default is 0.5]

   NOTES

   1. Pfields marked with '*' can receive updates from a table or real-time
      control source.  You can also update the wavetables (p6 and p7) using
      modtable(wavetable, "draw", ...).

   2. You can use most any mathematical expression to combine the two
      oscillator streams.  "a" represents the output of the 1st oscillator,
      "b", the second one.  The complete list of possibilities is given in
      "fparser.txt," the documentation for the wonderful math expression
      library that WAVY uses.  The library is by Juha Nieminen <warp at iki
      dot fi> and Joel Yliluoma.  (More at: http://iki.fi/warp/FunctionParser)
      If you create a very complicated expression, using trig functions, etc.,
      then this will not run very quickly.

   John Gibson, 6/15/05
*/
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <ctype.h>
#include <ugens.h>
#include <Ougens.h>
#include <PField.h>
#include "WAVY.h"
#include <rt.h>
#include <rtdefs.h>
#include <assert.h>
#include <string.h>

#define OPTIMIZE_EXPRESSION
//#define DEBUG


WAVY::WAVY()
	: _branch(0), _freqAraw(-DBL_MAX), _freqBraw(-DBL_MAX),
	  _phaseOffset(-DBL_MAX), _oscilA(NULL), _oscilB(NULL),
	  _combiner(NULL), _fp(NULL)
{
}


WAVY::~WAVY()
{
	delete _oscilA;
	delete _oscilB;
	delete _fp;
}


int WAVY::usage() const
{
	return die("WAVY",
	           "Usage: WAVY(start, dur, amp, freqA, freqB, phase_offset, "
	           "wavetableA, wavetableB, expression[, pan]");
}


int WAVY::setExpression()
{
	const PField &field = getPField(8);
	const char *fieldstr = field.stringValue(0.0);

	// copy expression string to <str>, stripping all white space
	const int len = strlen(fieldstr);
	char str[len + 1];
	char *p = str;
	for (int i = 0; i < len; i++) {
		const char c = fieldstr[i];
		if (!isspace(c))
			*p++ = c;
	}
	str[len] = 0;

	// Take care of some common simple cases first, since it's more efficient
	// to process them without using the fparser library.
	if (strcmp(str, "a") == 0)
		setCombineFunc(a);
	else if (strcmp(str, "b") == 0)
		setCombineFunc(b);
	else if (strcmp(str, "a+b") == 0)
		setCombineFunc(add);
	else if (strcmp(str, "a-b") == 0)
		setCombineFunc(subtract);
	else if (strcmp(str, "a*b") == 0)
		setCombineFunc(multiply);
	else {
		_fp = new FunctionParser();
		// use orig string so char offsets in err msg will be right
		int ret = _fp->Parse(fieldstr, "a,b");
		if (ret >= 0)
			return die("WAVY", "Parser error for expression \"%s\" at character "
			           "%d ('%c'): %s.",
			           fieldstr, ret, fieldstr[ret], _fp->ErrorMsg());
#ifdef OPTIMIZE_EXPRESSION
		_fp->Optimize();
#endif
	}

	return 0;
}


int WAVY::init(double p[], int n_args)
{
	if (n_args < 9)
		return usage();
	_nargs = n_args;

	const float outskip = p[0];
	const float dur = p[1];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (outputChannels() < 1 || outputChannels() > 2)
		return die("WAVY", "Must have mono or stereo output only.");

	int wavelenA;
	double *wavetabA = (double *) getPFieldTable(6, &wavelenA);
	if (wavetabA == NULL)
		return die("WAVY", "p6 must be wavetable (use maketable)");

	int wavelenB;
	double *wavetabB = (double *) getPFieldTable(7, &wavelenB);
	if (wavetabB == NULL) {
		wavetabB = wavetabA;
		wavelenB = wavelenA;
	}

	_oscilA = new Ooscil(SR, 440.0, wavetabA, wavelenA);
	_oscilB = new Ooscil(SR, 440.0, wavetabB, wavelenB);

	if (setExpression() != 0)
		return DONT_SCHEDULE;

	assert(_fp != NULL || _combiner != NULL);

	return nSamps();
}


void WAVY::doupdate()
{
	double p[10];
	update(p, 10, 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5 | 1 << 6 | 1 << 7 | 1 << 9);

	_amp = p[2];

	if (p[3] != _freqAraw) {
		_freqAraw = p[3];
		_freqA = (_freqAraw < 15.0) ? cpspch(_freqAraw) : _freqAraw;
		_oscilA->setfreq(_freqA);
		if (p[4] == 0.0) {
			_freqB = _freqA;
			_oscilB->setfreq(_freqB);
		}
	}

	if (p[4] != 0.0 && p[4] != _freqBraw) {
		_freqBraw = p[4];
		_freqB = (_freqBraw < 15.0) ? cpspch(_freqBraw) : _freqBraw;
		_oscilB->setfreq(_freqB);
	}

	if (p[5] != _phaseOffset) {
		_phaseOffset = p[5];
		// Get current phase of oscilA; offset oscilB phase from this by
		// _phaseOffset, in units relative to wavetable lengths.  We try to 
		// handle the case where the two freqs are different.
		const int lenB = _oscilB->getlength();
		const double phaseA = _oscilA->getphase() / _oscilA->getlength();
		const double freqscale = _freqB / _freqA;
		double phase = (phaseA + (_phaseOffset * freqscale)) * lenB;
		while (phase >= double(lenB))
			phase -= double(lenB);
		_oscilB->setphase(phase);
	}
	_pan = (_nargs > 9) ? p[9] : 0.5f;
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

		float sig1 = _oscilA->nexti();
		float sig2 = _oscilB->nexti();

		float out[chans];
		if (_fp)
			out[0] = eval(sig1, sig2) * _amp;
		else
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


Instrument *makeWAVY()
{
	WAVY *inst = new WAVY();
	inst->set_bus_config("WAVY");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("WAVY", makeWAVY);
}
#endif
