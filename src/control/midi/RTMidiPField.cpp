/* RTcmix - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

#include <RTMidiPField.h>
#include <RTcmixMIDI.h>
#include <PField.h>
#include <Ougens.h>
#include <assert.h>

extern int resetval;		// declared in src/rtcmix/minc_functions.c

#define LAGFACTOR 0.12
#define MAXCF     500.0
#define MINCF     0.05


RTMidiPField::RTMidiPField(
		RTcmixMIDI			*midiport,
		const double		minval,
		const double		maxval,
		const double		defaultval,
		const double		lag,
		const int			chan,
		const MIDIType		type,
		const MIDISubType	subtype)
	: RTNumberPField(0),
	  _midiport(midiport), _min(minval), _default(defaultval)
{
	assert(_midiport != NULL);

	_diff = maxval - minval;

	// Convert lag, a percentage in range [0, 100], to cutoff frequency,
	// depending on the control rate in effect when this PField was created.
	// Lag pct is inversely proportional to cf, because the lower the cf,
	// the longer the lag time.
	const double nyquist = resetval * 0.5;
	double cf = MAXCF * pow(2.0, -(lag * LAGFACTOR));
	if (cf <= MINCF)
		cf = MINCF;
	if (cf > nyquist)
		cf = nyquist;
	_filter = new Oonepole(resetval, cf);
}

RTMidiPField::~RTMidiPField() {}

double RTMidiPField::doubleValue(double dummy) const
{
	double val = computeValue();
	return _filter->next(val);
}

double RTMidiPField::computeValue() const
{
//FIXME
	return 0.0;
}

