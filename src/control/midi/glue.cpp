/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rtcmix_types.h>
#include <PField.h>
#include <RTMousePField.h>
#include <ugens.h>		// for warn, die


// -------------------------------------------------------- _midi_connection ---
//
//    midi = makeconnection("midi", min, max, default, lag, chan, type,
//                                                                [subtype])
//
//    <chan>      MIDI channel (1-16)
//
//    <type>      "noteon", "noteoff", "cntl", "prog", "bend", "monopress",
//                "polypress"
//
//    <subtype>   depends on <type>:
//                   noteon     "note", "velocity"
//                   noteoff    "note", "velocity"
//                   cntl       controller number or string symbol, such as
//                              "mod", "foot", "breath", "data", "volume", "pan"
//                   prog       not applicable
//                   bend       not applicable
//                   monopress  not applicable
//                   polypress  MIDI note number
//
//



static RTNumberPField *
_midi_usage()
{
	die("makeconnection (midi)",
		"Usage: makeconnection(\"midi\", min, max, default, lag, "
		"chan, type[, subtype])");
	return NULL;
}

static MIDIType
_convert_type(const char *type)
{
}

static MIDISubType
_convert_subtype(const MIDIType type, const char *subtype)
{
}

RTNumberPField *
midi_connection(const Arg args[], const int nargs)
{
	static RTcmixMIDI *mididriver = NULL;
	if (mididriver == NULL)					// first time, so init driver
		mididriver = createMIDIDriver();

	double minval, maxval, defaultval, lag;
	int chan;
	MIDIType type;
	MIDISubType subtype;

	if (args[1].isType(DoubleType))
		minval = args[1];
	else
		return _midi_usage();

	if (args[2].isType(DoubleType))
		maxval = args[2];
	else
		return _midi_usage();

	if (args[3].isType(DoubleType))
		defaultval = args[3];
	else
		return _midi_usage();

	if (args[4].isType(DoubleType))
		lag = args[4];
	else
		return _midi_usage();

	if (args[5].isType(DoubleType))
		chan = (int) args[5] - 1;		// convert to zero-based channel
	else
		return _midi_usage();

	if (args[6].isType(StringType))
		type = _convert_type(args[6]);
	else
		return _midi_usage();
	if (type == kMIDITypeInvalid)
		return _midi_usage();

	if (nargs > 7) {
		if (args[7].isType(StringType))
			subtype = _convert_subtype(type, args[7]);
		else if (args[7].isType(DoubleType))
//FIXME: not right: this is not a code, it's a literal int, e.g. controller num
			subtype = (MIDISubType) args[7];
		else
			return _midi_usage();
		if (subtype == kMIDISubTypeInvalid)
			return _midi_usage();
	}

	return new RTMIDIPField(mousewin, axis, minval, maxval, defaultval, lag,
										chan, type, subtype);
}

