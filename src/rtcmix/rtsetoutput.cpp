/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <ugens.h>
#include <iostream.h>
#include "Instrument.h"
#include "rtdefs.h"
#include "dbug.h"
#include <stdlib.h>

int Instrument::rtsetoutput(float start, float dur, Instrument *theInst)
{
	if (rtfileit < 0) {
		die(theInst->name(),
					"No output file open for this instrument (rtoutput failed?)!");
		return -1;
	}

	theInst->_start = start;
	theInst->_dur = dur;
	theInst->nsamps = (int) (dur * SR + 0.5);

	return 0;
}

