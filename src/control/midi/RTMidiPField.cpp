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
	  _midiport(midiport), _min(minval), _default(defaultval), _chan(chan),
	  _type(type), _subtype(subtype)
{
	assert(_midiport != NULL);

	_diff = maxval - minval;

	const double maxraw = (type == kMIDIPitchBendType) ? 16383.0 : 127.0;
	_factor = 1.0 / maxraw;

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
	int rawval;

	if (_type == kMIDIControlType)
		rawval = _midiport->getControl(_chan, _subtype);

	else if (_type == kMIDIPitchBendType)
		rawval = _midiport->getBend(_chan) + 8192;

	else if (_type == kMIDIChanPressType)
		rawval = _midiport->getChanPress(_chan);

	else if (_type == kMIDINoteOnPitchType)
		rawval = _midiport->getNoteOnPitch(_chan);
	else if (_type == kMIDINoteOnVelType)
		rawval = _midiport->getNoteOnVel(_chan);

	else if (_type == kMIDINoteOffPitchType)
		rawval = _midiport->getNoteOffPitch(_chan);
	else if (_type == kMIDINoteOffVelType)
		rawval = _midiport->getNoteOffVel(_chan);

	else if (_type == kMIDIPolyPressType)
		rawval = _midiport->getPolyPress(_chan, _subtype);

	else if (_type == kMIDIProgramType)
		rawval = _midiport->getProgram(_chan);

	else
		rawval = INVALID_MIDIVAL;

	if (rawval == INVALID_MIDIVAL)
		return _default;

	return _min + (_diff * (rawval * _factor));
}

