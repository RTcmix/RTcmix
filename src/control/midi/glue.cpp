/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <rtcmix_types.h>
#include <Option.h>
#include <PField.h>
#include <RTcmixMIDI.h>
#include <RTMidiPField.h>
#include <ugens.h>		// for warn, die

//FIXME: better to have a struct with one field a string, the other type?

#define NAME_VARIANTS	4
static char *_midi_type_name[][NAME_VARIANTS] = {
	// NB: Order of these must correspond to indices given by MIDIType enum!
	{ "cntl", "control", NULL, NULL },
	{ "noteonpitch", "onpitch", NULL, NULL },
	{ "noteonvel", "onvel", NULL, NULL },
	{ "noteoffpitch", "offpitch", NULL, NULL },
	{ "noteoffvel", "offvel", NULL, NULL },
	{ "bend", "pitchbend", NULL, NULL },
	{ "prog", "program", NULL, NULL },
	{ "chanpress", "monopress", "at", "aftertouch" },
	{ "polypress", "keypress", "polyat", "polyaftertouch" },
	{ NULL, NULL, NULL, NULL }
};

static char *_midi_controller_name[128] = {
				"", "mod", "breath", "", "foot",
				"port time", "data", "volume", "balance", "",
/* 10 */		"pan", "expression", "fxctl1", "fxctl2", "",
				"", "gp1", "gp2", "gp3", "gp4",
/* 20 */		"", "", "", "", "",
				"", "", "", "", "",
/* 30 */		"", "", "", "", "",
				"", "", "", "", "",
/* 40 */		"", "", "", "", "",
				"", "", "", "", "",
/* 50 */		"", "", "", "", "",
				"", "", "", "", "",
/* 60 */		"", "", "", "", "sustainsw",
				"portamentosw", "sostenutosw", "softpedsw", "legatosw", "hold2sw",
/* 70 */		"sc1", "sc2", "sc3", "sc4", "sc5",
				"sc6", "sc7", "sc8", "sc9", "sc10",
/* 80 */		"gp5", "gp6", "gp7", "gp8", "portamento",
				"", "", "", "", "",
/* 90 */		"", "fx1depth", "fx2depth", "fx3depth", "fx4depth",
				"fx5depth", "dataincr", "datadecr", "nrplsb", "nrpmsb",
/* 100 */	"rplsb", "rpmsb", "", "", "",
				"", "", "", "", "",
/* 110 */	"", "", "", "", "",
				"", "", "", "", "",
/* 120 */	"sound off", "reset cntl", "local cntl", "notes off", "omni off",
				"omni on", "mono on", "poly on"
};

// --------------------------------------------------------- midi_connection ---
//
//    midi = makeconnection("midi", min, max, default, lag, chan, type,
//                                                                [subtype])
//
//    <chan>      MIDI channel (1-16)
//
//    <type>      "noteonpitch", "noteonvel", "noteoffpitch", "noteoffpitch"
//                "cntl", "prog", "bend", "chanpress", "polypress"
//
//    <subtype>   depends on <type>:
//                   cntl       controller number or string symbol, such as
//                              "mod", "foot", "breath", "data", "volume", "pan"
//                   polypress  MIDI note number
//                   [no subtypes for the other types]
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
		for (int i = 0; i < 128; i++) {
			const char *name = _midi_controller_name[i];
			if (strcmp(subtype, name) == 0)
				return (MIDISubType) i;
		}
	}
	return kMIDIInvalidSubType;
}

/* Parse MIDI control name string, a sequence of colon-separated strings,
   with line-splicing recognition and some white-space eating.  Reads this:
  
      midi_control_names = " \
         bank select: \
         modulation wheel: \
         : \
         foot: \
         ...
         last name"

   resulting in these names: "bank select", "modulation wheel", "", "foot",
   ... "last name".  All except the empty "" override the builtin names.
*/
static void
_read_config()
{
#ifdef NOTYET
	char *names = strdup(Option::midiCntlNames());
	char *p, buf[128];
	int i = 0;
	int j = 0;
	bool eatspace = true;
	for (i = 0, p = names; i < 128 && *p != 0; p++) {
		if (eatspace && isspace(*p))
			continue;
		if (*p == '\\')
			continue;
		if (*p == ':') {
			buf[j] = 0;
			if (buf[0])
				_midi_controller_name[i] = strdup(buf);
			i++;
			j = 0;
			eatspace = true;
		}
		else {
			buf[j++] = *p;
			if (j == 127) {
				warn("makeconnection (midi)",
						"Controller name in config file too long");
				break;
			}
			eatspace = false;
		}
	}
	free(names);
#endif
}

RTNumberPField *
midi_connection(const Arg args[], const int nargs)
{
	static bool configRead = false;

	static RTcmixMIDI *midiport = NULL;
	if (midiport == NULL)				// first time, so init midi system
		midiport = createMIDIPort();
	if (midiport == NULL)
		return NULL;

	if (configRead == false) {
		_read_config();
		configRead = true;
	}

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
	if (lag < 0.0 || lag > 100.0) {
		die("makeconnection (midi)", "<lag> must be between 0 and 100");
		return NULL;
	}

	if (args[5].isType(DoubleType))
		chan = (int) args[5] - 1;		// convert to zero-based channel
	else
		return _midi_usage();
	if (chan < 0 || chan > 15) {
		die("makeconnection (midi)", "<chan> must be between 1 and 16");
		return NULL;
	}

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

