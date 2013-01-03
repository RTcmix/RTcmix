// Copyright (C) 2005 John Gibson.  See ``LICENSE'' for the license to this
// software and for a DISCLAIMER OF ALL WARRANTIES.

/* SPECTEQ2 - FFT-based EQ

   Parameters marked with '*' can receive dynamic updates from a table or
   real-time control source.  See note 1 below.

     p0  = output start time
     p1  = input start time
     p2  = duration
   * p3  = amplitude multiplier

     p4  = FFT length (power of 2, usually 1024)
     p5  = window length (power of 2, usually FFT length * 2)
     p6  = window table (or zero for internally generated Hamming window)
     p7  = overlap - how much FFT windows overlap (positive power of 2)
           1: no overlap, 2: hopsize=FFTlen/2, 4: hopsize=FFTlen/4, etc.
           2 or 4 is usually fine; 1 is fluttery; higher overlaps use more CPU.

   * p8  = EQ table (i.e., amplitude scaling of each band), in dB (0 dB means
           no change, + dB boost, - dB cut).

   The next two parameters define the range within which the EQ table functions.
   See note (3) below for more information.

   * p9  = minimum frequency [optional, default is 0 Hz]
   * p10 = maximum frequency [optional, default is Nyquist] 
           If zero, max. freq. will be set to the Nyquist frequency.

     p11 = bin-mapping table -- gives the number of adjacent FFT bins to affect
           for each of the elements in the EQ control table.  To ignore this,
           just pass a zero here.  See note 4 for tips on using this feature.
           [optional, default is zero]

   * p12 = bypass (0: not bypassed, 1: bypassed) [optional, default is 0]
     p13 = input channel [optional, default is 0]
   * p14 = pan (in percent-to-left format) [optional, default is .5]

   NOTES:

   1. p3 (amp), p9 (min. freq.), p10 (max. freq.), p12 (bypass) and p14 (pan)
      can receive dynamic updates from a table or real-time control source.
      Note that updating min. or max. freq. can be rather expensive.

      It is also possible to update the EQ table (p8) dynamically using
      modtable(table, "draw", ...).

   2. Output begins after a brief period of time during which internal buffers
      are filling.  This time is the duration corresponding to the following
      number of sample frames:  window length - (fft length / overlap).  This
      duration is called "latency duration" below.

   3. Frequency range (p9-10) for EQ table

      If both min. and max. frequency values are zero (i.e., max. is Nyquist),
      and the EQ table is sized to half the FFT length (p4), then the instrument
      behaves similarly to SPECTEQ v1.  That is, each table element controls one
      FFT bin.  If the control table is larger than half the FFT length, then
      the extra values at the end of the table are ignored (and a warning
      printed).  If the table is less than half the FFT length, then the scheme
      described below applies.

      In all other cases, the first element of a table controls all FFT bins
      below and including the minimum frequency.  Successive table elements
      control groups of bins above this frequency.  The last element of the
      table controls all FFT bins at and above the maximum frequency.  So you
      can think of the first table element as a low shelf (brick wall) filter
      cutoff frequency, and the last element as a high shelf filter cutoff
      frequency.  Interior elements are like peak/notch filters.  If the EQ
      table has too many elements, then the extra values at the end of the
      table are ignored.

      If the EQ table is smaller than the number of FFT bins it affects, then
      the table elements are mapped to FFT bins in a particular way.  The method
      used creates greater resolution for lower frequencies.  For example, if
      there are 512 FFT bins (i.e., half the FFT length), but the EQ table has
      only 32 elements, then there is a one-to-one mapping from table elements
      to bins for the lower frequencies.  For the higher frequencies, one table
      element might control 30 or more bins.

   4. If using the bin-mapping table (p11), it must be the same size as the EQ
      table (p8).  If the sum of all the elements of the mapping table is less
      than the total number of FFT bins -- which is (FFT size / 2) + 1, and
      includes 0 Hz and Nyquist bins) -- then the extra higher-frequency bins
      will be assigned to the last EQ table element.  If the sum of mapping
      table elements is greater than the number of bins, then some higher-
      frequency bins will be omitted.  The frequency ranges discussed in
      note 3 are ignored when using the bin-mapping table.  Don't you wish you
      had passed zero here to ignore this feature?


   John Gibson <johgibso at indiana dot edu>, 6/12/05.
*/

//#define DUMP
//#define DEBUG
#include "SPECTEQ2.h"

// This inst is just a stripped down version of SPECTACLE2.cpp.  -JGG

// ----------------------------------------------------------------- SPECTEQ2 --
SPECTEQ2::SPECTEQ2()
	: _eqconst(0.0f)
{
}


// ---------------------------------------------------------------- ~SPECTEQ2 --
SPECTEQ2::~SPECTEQ2()
{
	// NB: we don't own the EQ table.
}


// -------------------------------------------------------------------- usage --
int SPECTEQ2::usage()
{
	return die(NULL,
		"Usage: %s(start, inskip, indur, amp, fftlen, windowlen, windowtype, "
		"overlap, eqtable, minfreq, maxfreq, binmaptable, bypass, inchan, pan)",
		instname());
}


// ------------------------------------------------------------------ getargs --
// Grab initial values of pfields that are not dynamically updateable or that
// have optional non-zero values.  outskip, inskip, dur, out amp -- as well as
// verification for all others that are not specific to this subclass -- are
// handled in base class init.  ringdur init can wait until subinit, after
// FFT parameters are set.

int SPECTEQ2::getargs(double p[], int n_args)
{
	if (n_args < 9)
		return usage();

	set_fftlen(int(p[4]));
	set_window_len(int(p[5]));
	set_window_pfield_index(6);
	set_overlap(int(p[7]));
	set_inchan(int(p[13]));
	set_pan(0.5f);			// default, maybe overridden in doupdate

	return 0;
}


// ------------------------------------------------------------------ subinit --
// Perform any initialization specific to this subclass.  Make sure ringdur
// is set before return.

int SPECTEQ2::subinit(double p[], int n_args)
{
	int eqtablen;
	_eqtable = (double *) getPFieldTable(8, &eqtablen);
	if (!_eqtable)
		_eqconst = p[8];
	else
		_control_table_size = eqtablen;

	int len;
	double *binmaptable = (double *) getPFieldTable(11, &len);
	if (binmaptable) {
		if (len != eqtablen)
			die(instname(), "The bin-mapping table (p11) must be the same size as "
			                "the EQ table (p8).");
		set_binmap_table(binmaptable);
		if (p[9] != 0.0 || (p[10] != 0.0 || p[10] != _nyquist))
			rtcmix_warn(instname(), "Use of the bin-mapping table ignores the freq. "
			                 "range set in p9-10.");
	}

	set_ringdur(0.0f);

	return 0;
}


// ------------------------------------------------------------- subconfigure --
int SPECTEQ2::subconfigure()
{
	return 0;
}


// ---------------------------------------------------------------- subupdate --
void SPECTEQ2::subupdate()
{
	// NB: update EQ table, so that live table draw will work
	double p[15];
	update(p, 15, 1 << 3 | 1 << 8 | 1 << 9 | 1 << 10 | 1 << 12 | 1 << 14);

	set_oamp(p[3]);
	set_freqrange(p[9], p[10], "EQ");
	set_wet(bool(p[12]) ? 0.0f : 1.0f);
	if (_nargs > 14)
		set_pan(p[14]);
}


// ----------------------------------------------------------- dump_anal_bins --
void SPECTEQ2::dump_anal_bins()
{
#ifdef DUMP
	printf("\n----------------------------------------\n");
	for (int i = 0; i <= _half_fftlen; i++) {
		int index = i << 1;
		printf("%3d [%.3f Hz]:\tmag=%f, phs=%f\n",
					i,
					i * _fund_anal_freq,
					_anal_bins[index],
					_anal_bins[index + 1]);
	}
#endif
}


// ---------------------------------------------------------- modify_analysis --
void SPECTEQ2::modify_analysis(bool reading_input)
{
	DPRINT1("modify_analysis: .............. reading_input=%d\n", reading_input);
#ifdef DUMP
	dump_anal_bins();
#endif

	for (int i = 0; i <= _half_fftlen; i++) {
		int index = i << 1;
		float eq = _eqtable ? _eqtable[_bin_groups[i]] : _eqconst;
		float mag = reading_input ? (_anal_bins[index] * _ampdb(eq)) : 0.0f;
		_anal_bins[index] = mag;
	}
}


// ------------------------------------------------------------- makeSPECTEQ2 --
Instrument *makeSPECTEQ2()
{
	SPECTEQ2 *inst;

	inst = new SPECTEQ2();
	inst->set_bus_config("SPECTEQ2");

	return inst;
}

#ifndef MAXMSP
// ---------------------------------------------------------------- rtprofile --
void rtprofile()
{
	RT_INTRO("SPECTEQ2", makeSPECTEQ2);
}
#endif

