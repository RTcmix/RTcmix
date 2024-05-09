/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
 See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
 the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
 */
//
//  SYSEX.cpp
//
//  Created by Douglas Scott on 1/19/20.
//
/*
 p0 = output start time
 p1 = output duration
 p2 = Sysex message (as string containing space-separated hex values)
*/

#include "SYSEX.h"
#include "RTMIDIOutput.h"
#include <ugens.h>
#include <rt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG 0

#if DEBUG
#define PRINT printf
#else
#define PRINT if (0) printf
#endif

SYSEX::SYSEX() : MIDIBase(), _mesg(0)
{
}

SYSEX::~SYSEX()
{
	delete [] _mesg;
}

int SYSEX::init(double p[], int n_args)
{
    // NOTE: Because SYSEX does not have a channel field, we have to fake out the base class here
    double bp[3];  bp[0] = p[0]; bp[1] = p[1]; bp[2] = 0;
    if (MIDIBase::init(bp, 3) < 0) {
        return DONT_SCHEDULE;
    }
	char *bytes = DOUBLE_TO_STRING(p[2]), *token;
    if (bytes == NULL) {
        return die("SYSEX", "Illegal system exclusive value string");
    }
	_mesg = new unsigned char [(strlen(bytes) + 2) / 3];	// "XX YY ZZ" etc
	int n = 0;
	while ((token = strsep(&bytes, " ")) != NULL) {
		unsigned char hexval = strtol(token, NULL, 16) & 0xff;
//		printf("%s -> %X, ", token, hexval);
		_mesg[n++] = hexval;
	}
	printf("\n");

    return nSamps();
}

void SYSEX::doStart(FRAMETYPE frameOffset)
{
    long timestamp = getEventTimestamp(frameOffset);
    PRINT("doStart sending system exclusive at timestamp %ld\n", timestamp);
    _outputPort->sendSysEx(timestamp, _mesg);
}

Instrument *makeSYSEX()
{
    SYSEX *inst = new SYSEX();
    inst->set_bus_config("SYSEX");
    
    return inst;
}
