/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <rtcmix_types.h>
#include <PField.h>
#include <RTcmixMIDI.h>
#include <RTMidiPField.h>
#include <ugens.h>		// for warn, die

//FIXME: better to have a struct with one field a string, the other type

#define NAME_VARIANTS 4
static char *_midi_type_name[][NAME_VARIANTS] = {
	// NB: Order of these must correspond to indices given by MIDIType enum!
	{ "cntl", "control", NULL, NULL },
	{ "noteon", NULL, NULL, NULL },
	{ "noteoff", NULL, NULL, NULL },
	{ "bend", "pitchbend", NULL, NULL },
	{ "prog", "program", NULL, NULL },
	{ "chanpress", "monopress", "at", "aftertouch" },
	{ "polypress", "keypress", "polyat", "polyaftertouch" },
	{ NULL, NULL, NULL, NULL }
};

static char *_midi_controller_name[][NAME_VARIANTS] = {
	{ "banksel", NULL, NULL, NULL },
	{ "mod", "modwheel", "modulation", NULL },
	{ "breath", NULL, NULL, NULL },
	{ NULL, NULL, NULL, NULL },
	{ "foot", NULL, NULL, NULL },
	{ "port time", "portamento time", NULL, NULL },
	{ "data", NULL, NULL, NULL },
	{ "vol", "volume", NULL, NULL },
	{ "bal", "balance", NULL, NULL },
	{ NULL, NULL, NULL, NULL },
	{ "pan", NULL, NULL, NULL },
	{ "exp", "expression", NULL, NULL },
	{ NULL, NULL, NULL, NULL }
//FIXME: add others
};

// -------------------------------------------------------- _midi_connection ---
//
//    midi = makeconnection("midi", min, max, default, lag, chan, type,
//                                                                [subtype])
//
//    <chan>      MIDI channel (1-16)
//
//    <type>      "noteon", "noteoff", "cntl", "prog", "bend", "chanpress",
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
_string_to_type(const char *type)
{
	for (int i = 0; _midi_type_name[i][0] != NULL; i++) {
		for (int j = 0; j < NAME_VARIANTS; j++) {
			const char *name = _midi_type_name[i][j];
			if (name == NULL)
				break;
			if (strcasecmp(type, name) == 0)
				return (MIDIType) i;
		}
	}
	return kMIDIInvalidType;
}

static MIDISubType
_string_to_subtype(const MIDIType type, const char *subtype)
{
	if (type == kMIDIControlType) {
		for (int i = 0; _midi_controller_name[i][0] != NULL; i++) {
			for (int j = 0; j < NAME_VARIANTS; j++) {
				const char *name = _midi_controller_name[i][j];
				if (name == NULL)
					break;
				if (strcasecmp(type, name) == 0)
					return (MIDISubType) i;
			}
		}
	}
	else if (type == kMIDINoteOnType || type == kMIDINoteOffType) {
		if (strcmp(subtype, "note") == 0)
			return kMIDINoteSubType;
		else if (strncmp(subtype, "vel", 3) == 0)
			return kMIDIVelocitySubType;
	}
	return kMIDIInvalidSubType;
}

RTNumberPField *
midi_connection(const Arg args[], const int nargs)
{
	static RTcmixMIDI *midiport = NULL;
	if (midiport == NULL)				// first time, so init midi system
		midiport = createMIDIPort();

	double minval, maxval, defaultval, lag;
	int chan;
	MIDIType type = kMIDIInvalidType;
	MIDISubType subtype = kMIDIInvalidSubType;

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
		type = _string_to_type(args[6]);
	else
		return _midi_usage();
	if (type == kMIDIInvalidType)
		return _midi_usage();

	if (nargs > 7) {
		if (args[7].isType(StringType))
			subtype = _string_to_subtype(type, args[7]);
		else if (args[7].isType(DoubleType))
			// NB: this can be a code or a literal int, e.g. note or controller num
			subtype = (MIDISubType) (int) args[7];
		else
			return _midi_usage();
		if (subtype == kMIDIInvalidSubType)
			return _midi_usage();
	}

	return new RTMidiPField(midiport, minval, maxval, defaultval, lag, chan,
																		type, subtype);
}

