/* RTcmix - Copyright (C) 2000 The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* The set_option function, called from a script, lets the user override
   default options (and those stored in the .rtcmixrc file).  The options
   are kept in the <options> object (see Option.h).      -JGG, 6/30/04
*/
#include <RTcmix.h>
#include <ugens.h>		// for die()
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <Option.h>

enum ParamType {
	DEVICE,
	INDEVICE,
	OUTDEVICE,
	MIDI_INDEVICE,
	MIDI_OUTDEVICE,
	BUFFER_COUNT,
	AUDIO,
	RECORD,
	CLOBBER,
	REPORT_CLIPPING,
	CHECK_PEAKS,
	FULL_DUPLEX,
};

#define OPT_STRLEN 128

struct Param {
	char arg[OPT_STRLEN];	 
	ParamType type;
	bool value;		// use false if not relevant, i.e. for key=value style
};

static Param param_list[] = {
	{ "DEVICE", DEVICE, false},
	{ "INDEVICE", INDEVICE, false},
	{ "OUTDEVICE", OUTDEVICE, false},
	{ "MIDI_INDEVICE", MIDI_INDEVICE, false},
	{ "MIDI_OUTDEVICE", MIDI_OUTDEVICE, false},
	{ "BUFFER_COUNT", BUFFER_COUNT, false},
	{ "AUDIO_ON", AUDIO, true},
	{ "AUDIO_OFF", AUDIO, false},
	{ "RECORD_ON", RECORD, true},
	{ "RECORD_OFF", RECORD, false},
	{ "PLAY_ON", AUDIO, true},
	{ "PLAY_OFF", AUDIO, false},
	{ "CLOBBER_ON", CLOBBER, true},
	{ "CLOBBER_OFF", CLOBBER, false},
	{ "REPORT_CLIPPING_ON", REPORT_CLIPPING, true},
	{ "REPORT_CLIPPING_OFF", REPORT_CLIPPING, false},
	{ "CHECK_PEAKS_ON", CHECK_PEAKS, true},
	{ "CHECK_PEAKS_OFF", CHECK_PEAKS, false},
	{ "FULL_DUPLEX_ON", FULL_DUPLEX, true},
	{ "FULL_DUPLEX_OFF", FULL_DUPLEX, false},
};
static int num_params = sizeof(param_list) / sizeof(Param);

double RTcmix::set_option(float *p, int nargs, double pp[])
{
	int  j;
	char opt[OPT_STRLEN];

	for (int i = 0; i < nargs; i++) {
		int matched;
		int space_state;

		char *p, *arg = (char *) ((int) pp[i]);		// cast pfield to string

		// Strip white-space chars from text to the left of any '=' and between
		// the '=' and the next non-white space.  (The reason is to preserve
		// options that must include spaces, such as "MOTU 828".)  Store result
		// into <opt>.
		int len = arg ? strlen(arg) : 0;
		if (len > OPT_STRLEN - 1)
			len = OPT_STRLEN - 1;
		space_state = 0;
		for (j = 0, p = opt; j < len; j++) {
			if (space_state > 1)
				*p++ = arg[j];
			else if (!isspace(arg[j])) {
				if (space_state == 1)
					space_state++;
				else if (arg[j] == '=')
					space_state = 1;
				*p++ = arg[j];
			}
		}
		*p = '\0';

		// Two styles of option string: a single "value" and a "key=value" pair.
		matched = 0;
		p = strchr(opt, '=');					// check for "key=value"
		if (p) {
			*p++ = '\0';						// <opt> is now key only
			if (*p == '\0') {
				 die("set_option", "Missing value for key \"%s\"", opt);
				 return -1.0;
			}
			// p now points to value string
			for (j = 0; j < num_params; j++) {
				if (strcasecmp(param_list[j].arg, opt) == 0) {
					matched = 1;
					break;
				}
			}
		}
		else {									// check for single "value"
			 for (j = 0; j < num_params; j++) {
				 if (strcasecmp(param_list[j].arg, opt) == 0) {
					 matched = 1;
					 break;
				 }
			 }
		}
		if (!matched) {
			die("set_option", "Unrecognized argument \"%s\"", opt);
			return -1.0;
		}
		
		switch (param_list[j].type) {
		case DEVICE:
			if (p == NULL) {
				 die("set_option", "No value for \"device\"");
				 return -1.0;
			}
			Option::device(p);
			break;
		case INDEVICE:
			if (p == NULL) {
				 die("set_option", "No value for \"indevice\"");
				 return -1.0;
			}
			Option::inDevice(p);
			break;
		case OUTDEVICE:
			if (p == NULL) {
				 die("set_option", "No value for \"outdevice\"");
				 return -1.0;
			}
			Option::outDevice(p);
			break;
		case MIDI_INDEVICE:
			if (p == NULL) {
				 die("set_option", "No value for \"midi_indevice\"");
				 return -1.0;
			}
			Option::midiInDevice(p);
			break;
		case MIDI_OUTDEVICE:
			if (p == NULL) {
				 die("set_option", "No value for \"midi_outdevice\"");
				 return -1.0;
			}
			Option::midiOutDevice(p);
			break;
		case BUFFER_COUNT:
			if (p == NULL) {
				 die("set_option", "No value for \"buffer_count\"");
				 return -1.0;
			}
			else {
				int count = atoi(p);
				if (count <= 0) {
					 die("set_option", "\"buffer_count\" value must be > 0");
					 return -1.0;
				}
				Option::bufferCount(count);
			}
			break;
		case AUDIO:
			Option::play(param_list[j].value);
			break;
		case RECORD:
			Option::record(param_list[j].value);
			break;
		case CLOBBER:
			Option::clobber(param_list[j].value);
			break;
		case REPORT_CLIPPING:
			Option::reportClipping(param_list[j].value);
			break;
		case CHECK_PEAKS:
			Option::checkPeaks(param_list[j].value);
			break;
		case FULL_DUPLEX:
			bool full_duplex = param_list[j].value;
			if (full_duplex && rtsetparams_called) {
				die("set_option", "Turn on full duplex BEFORE calling rtsetparams.");
				return -1.0;
			}
			// The full duplex state has now been broken up into the <play> and
			// <record> options, used during audio setup. rtsetparams() checks
			// <record>.
			if (full_duplex)
				Option::record(true);
			else {
				// If not play, then record.
				bool state = Option::record() && !Option::play();
				Option::record(state);
			}
			// Same check as above, for record.
			if (Option::record() && rtsetparams_called) {
				die("set_option", "Turn on record BEFORE calling rtsetparams.");
				return -1.0;
			}
			break;
		}
	}
	return 0.0;
}

