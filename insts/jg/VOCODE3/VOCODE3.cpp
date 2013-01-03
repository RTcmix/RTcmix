// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

/* VOCODE3 - another channel vocoder

   Performs a filter-bank analysis of the right input channel (the modulator),
   and uses the time-varying energy measured in the filter bands to control
   a corresponding filter bank that processes the left input channel (the
   carrier).  You can configure independently the two filter banks, and you
   can map modulator bands to carrier bands freely.

      p0  = output start time
      p1  = input start time (must be 0 for aux bus)
      p2  = duration
    * p3  = amplitude multiplier (post-processing)

      The following two tables give center frequencies for the bandpass
      filters.  The tables must be the same size; the size determines the
      number of filters.  You can pass the same table to both pfields.
      Try using the "literal" variant of maketable to create the tables.

    * p4  = table of modulator center frequencies (Hz or linear octaves)
    * p5  = table of carrier center frequencies (Hz or linear octaves)

    * p6  = table mapping modulator to carrier filter bands (see note 3)
            If you don't want to think about this, pass a zero here.
    * p7  = carrier amplitude scaling table (see note 4)
            If you don't want this scaling, pass a zero here.

    * p8  = transposition of modulator center frequencies (linear octaves only)
    * p9  = transposition of carrier center frequencies (linear octaves only)

    * p10 = modulator filter Q (i.e., cf / bandwidth in Hz)
    * p11 = carrier filter Q

    * p12 = filter response time (seconds) [optional, default is 0.01]
            Determines how often changes in modulator power are measured.
    * p13 = hold -- 0: normal, 1: modulator is disengaged and has no further
            effect on the carrier.  [optional, default is 0]

    * p14 = pan (in percent-to-left-channel form) [optional, default is 0.5]

   Parameters marked with '*' can receive dynamic updates from a table or
   real-time control source.  In the case of the center frequency tables (p4-5)
   and mapping table (p6), this can be done via modtable(..., "draw", ...).

   NOTES

     1. Currently in RTcmix it's not possible for an instrument to take input
        from both an "in" bus and an "aux in" bus at the same time.  So if you
        want the modulator to come from a microphone -- which must enter via an
        "in" bus -- and the carrier to come from a WAVETABLE instrument via an
        "aux" bus, then you must route the mic into the MIX instrument as a way
        to convert it from "in" to "aux in".

     2. The "left" input channel comes from the bus with the lower number;
        the "right" input channel from the bus with the higher number.

     3. The table mapping modulator to carrier filter bands (p6) must be the
        same size as the two center frequency tables.  The indices of the table
        represent modulator bands; the values of the table represent carrier
        bands.  So a table of { 2, 3, 0, 1 } connects the 0th modulator band to
        the 2nd carrier band, the 1st mod. to the 3rd car., the 2nd mod. to the
        0th car. and the 3rd mod. to the 1st car.  Note that more than one
        modulator band may map to the same carrier band, and that (in this case)
        a carrier band may have no modulator input.  Pass zero to get the
        default linear mapping: 0->0, 1->1, 2->2, 3->3, etc.

     4. The carrier amplitude scaling table must be the same size as the
        table of carrier center frequencies.  Each element is a linear amplitude
        scaling factor applied to the corresponding carrier band output.


   John Gibson <johgibso at indiana dot edu>, 6/19/05.
*/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <ugens.h>
#include <Ougens.h>
#include "VOCODE3.h"
#include <rt.h>
#include <rtdefs.h>

const OeqType kBandPassType = OeqBandPassCPG;  // constant 0 dB peak gain


VOCODE3::VOCODE3()
	: _branch(0), _numfilts(0), _hold(0), _maptable(NULL),
	  _responsetime(-FLT_MAX), _nyquist(SR * 0.5f),
	  _in(NULL), _lastmod(NULL), _modulator_filt(NULL), _carrier_filt(NULL),
	  _balancer(NULL)
{
}


VOCODE3::~VOCODE3()
{
	delete [] _modtable_prev;
	delete [] _cartable_prev;
	delete [] _maptable;
	delete [] _in;
	delete [] _lastmod;

	for (int i = 0; i < _numfilts; i++) {
		delete _modulator_filt[i];
		delete _carrier_filt[i];
		delete _balancer[i];
	}
	delete [] _modulator_filt;
	delete [] _carrier_filt;
	delete [] _balancer;
}


int VOCODE3::usage()
{
	return die("VOCODE3", "Usage: "
	           "VOCODE3(start, inskip, dur, amp, modtable, cartable, maptable, "
	           "modtransp, cartransp, modq, carq, response_time[, pan])");
}

int VOCODE3::init(double p[], int n_args)
{
	_nargs = n_args;
	if (_nargs < 11)
		return usage();

	const float outskip = p[0];
	const float inskip = p[1];
	const float dur = p[2];

	if (rtsetoutput(outskip, dur, this) == -1)
		return DONT_SCHEDULE;
	if (rtsetinput(inskip, this) == -1)
		return DONT_SCHEDULE;

	if (outputChannels() > 2)
		return die("VOCODE3", "Output must be either mono or stereo.");
	if (inputChannels() != 2)
		return die("VOCODE3",
		"Must use 2 input channels: 'left' for carrier; 'right' for modulator.");

	_modtable_src = (double *) getPFieldTable(4, &_numfilts);
	if (_modtable_src == NULL)
		return die("VOCODE3", "p4 must have the modulator center freq. table.");
	int len;
	_cartable_src = (double *) getPFieldTable(5, &len);
	if (_cartable_src == NULL)
		return die("VOCODE3", "p5 must have the carrier center freq. table.");
	if (len != _numfilts)
		return die("VOCODE3", "Modulator and carrier center freq. tables must "
									 "be the same size.");
	_modtable_prev = new double [_numfilts];   // these two arrays inited below
	_cartable_prev = new double [_numfilts];

	_maptable_src = (double *) getPFieldTable(6, &len);
	if (_maptable_src && (len != _numfilts))
		return die("VOCODE3", "Center freq. mapping table (p6) must be the same "
									 "size as the modulator and carrier tables.");
	_maptable = new int [_numfilts];
	if (_maptable_src) {
		for (int i = 0; i < _numfilts; i++)
			_maptable[i] = int(_maptable_src[i]);
	}
	else {	// no user mapping table; make linear mapping
		for (int i = 0; i < _numfilts; i++)
			_maptable[i] = i;
	}

	_scaletable = (double *) getPFieldTable(7, &len);
	if (_scaletable && (len != _numfilts))
		return die("VOCODE3", "The carrier scaling table must be the same size "
		                      "(%d elements) as the carrier frequency table.",
		                      _numfilts);

	_modtransp = p[8];
	_cartransp = p[9];
	_modq = p[10];
	_carq = p[11];

	_lastmod = new float [_numfilts];

	_modulator_filt = new Oequalizer * [_numfilts];
	_carrier_filt = new Oequalizer * [_numfilts];
	_balancer = new Obalance * [_numfilts];

#ifdef NOTYET
	const bool print_stats = Option::printStats();
#else
	const bool print_stats = true;
#endif
	if (print_stats) {
		rtcmix_advise(NULL, "VOCODE3:  mod. CF\tcar. CF  [Hz, after transp]");
		rtcmix_advise(NULL, "          (Q=%2.1f)\t(Q=%2.1f)", _modq, _carq);
		rtcmix_advise(NULL, "          -----------------------------------------");
	}

	for (int i = 0; i < _numfilts; i++) {
		_modulator_filt[i] = new Oequalizer(SR, kBandPassType);
		_modtable_prev[i] = _modtable_src[i];
		float mfreq = updateFreq(_modtable_src[i], _modtransp);
		_modulator_filt[i]->setparams(mfreq, _modq);

		_carrier_filt[i] = new Oequalizer(SR, kBandPassType);
		_cartable_prev[i] = _cartable_src[i];
		float cfreq = updateFreq(_cartable_src[i], _cartransp);
		_carrier_filt[i]->setparams(cfreq, _carq);

		_balancer[i] = new Obalance(SR);

		_lastmod[i] = 0.0f;	// not necessary

		if (print_stats)
			rtcmix_advise(NULL, "          %7.1f\t%7.1f", mfreq, cfreq);
	}

	return nSamps();
}


int VOCODE3::configure()
{
	_in = new float [RTBUFSAMPS * inputChannels()];
	return _in ? 0 : -1;
}


void VOCODE3::doupdate()
{
	double p[15];
	update(p, 15);

	_amp = p[3];

	bool setallmodfilts = false;
	bool setallcarfilts = false;

	if (p[10] != _modq) {
		_modq = p[10];
		setallmodfilts = true;
	}
	if (p[11] != _carq) {
		_carq = p[11];
		setallcarfilts = true;
	}

	if (p[8] != _modtransp) {
		_modtransp = p[8];
		setallmodfilts = true;
	}
	if (p[9] != _cartransp) {
		_cartransp = p[9];
		setallcarfilts = true;
	}

	if (setallmodfilts) {
		for (int i = 0; i < _numfilts; i++) {
			_modtable_prev[i] = _modtable_src[i];     // src freq may've changed
			float freq = updateFreq(_modtable_src[i], _modtransp);
			_modulator_filt[i]->setparams(freq, _modq);
		}
	}
	else if (p[4] != 0.0) {                // mod. cf table has changed
		for (int i = 0; i < _numfilts; i++) {
			if (_modtable_prev[i] != _modtable_src[i]) {
				_modtable_prev[i] = _modtable_src[i];
				float freq = updateFreq(_modtable_src[i], _modtransp);
				_modulator_filt[i]->setparams(freq, _modq);
			}
		}
	}

	if (setallcarfilts) {
		for (int i = 0; i < _numfilts; i++) {
			_cartable_prev[i] = _cartable_src[i];
			float freq = updateFreq(_cartable_src[i], _cartransp);
			_carrier_filt[i]->setparams(freq, _carq);
		}
	}
	else if (p[5] != 0.0) {                // car. cf table has changed
		for (int i = 0; i < _numfilts; i++) {
			if (_cartable_prev[i] != _cartable_src[i]) {
				_cartable_prev[i] = _cartable_src[i];
				float freq = updateFreq(_cartable_src[i], _cartransp);
				_carrier_filt[i]->setparams(freq, _carq);
			}
		}
	}

	if (p[6] != 0.0) {                     // mapping table has changed
		for (int i = 0; i < _numfilts; i++)
			_maptable[i] = int(_maptable_src[i]);
	}

	const float rt = (_nargs > 12) ? p[12] : 0.01f;
	if (rt != _responsetime) {
		_responsetime = rt;
		int windowlen = int(_responsetime * SR + 0.5f);
		if (windowlen < 2)   // otherwise, can get ear-splitting output
			windowlen = 2;
		for (int i = 0; i < _numfilts; i++)
			_balancer[i]->setwindow(windowlen);
	}

	int newhold = int(p[13]);
	if (newhold != _hold) {
		_hold = newhold;
		if (_hold) {
			for (int i = 0; i < _numfilts; i++) {
				_lastmod[i] = _modulator_filt[i]->last();
				// _modulator_filt[i]->clear();	// doesn't seem necessary
			}
		}
	}

	_pan = (_nargs > 14) ? p[14] : 0.5f;
}


int VOCODE3::run()
{
	const int inchans = inputChannels();
	const int samps = framesToRun() * inchans;
	rtgetin(_in, this, samps);

	for (int i = 0; i < samps; i += inchans) {
		if (--_branch <= 0) {
			doupdate();
			_branch = getSkip();
		}
		const float carsig = _in[i];
		const float modsig = _in[i + 1];

		float out[2];
		out[0] = 0.0f;
		for (int j = 0; j < _numfilts; j++) {
			float mod;
			if (_hold)
				mod = _lastmod[j];
			else
				mod = _modulator_filt[j]->next(modsig);
			float car = _carrier_filt[_maptable[j]]->next(carsig);
			float balsig = _balancer[j]->next(car, mod);
			if (_scaletable != NULL)
				balsig *= _scaletable[j];
			out[0] += balsig;
		}

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


Instrument *makeVOCODE3()
{
	VOCODE3 *inst = new VOCODE3();
	inst->set_bus_config("VOCODE3");

	return inst;
}

#ifndef MAXMSP
void rtprofile()
{
	RT_INTRO("VOCODE3", makeVOCODE3);
}
#endif
