#include <iostream.h>
#include <stdio.h>
#include "../rtstuff/Instrument.h"
#include "../rtstuff/rtdefs.h"

/* these are defined in rtinput.c */
extern InputDesc inputFileTable[];
extern off_t rtInitialOffset;
extern int rtInputIndex;
extern double inSR;
extern int inNCHANS;
extern int input_on;


int rtsetinput(float start, Instrument *theInst)
{
	if (input_on) { /* take from input device, not file */
		theInst->inputsr = inSR;
		theInst->inputchans = inNCHANS;
		theInst->fdIndex = AUDIO_DEVICE;
		return(1);
	}

	/* file was opened in rtinput().  Here we simply
	 * store the index into the inputFileTable for the file.
	 * We also increment the reference count for this file.
	 */

	theInst->fdIndex = rtInputIndex;

	/* check to be sure the open was successful */

	if (theInst->fdIndex < 0 || inputFileTable[theInst->fdIndex].fd < 1) {
		fprintf(stderr, "No input file open for this instrument!\n");
		return(0);
	}

	inputFileTable[theInst->fdIndex].refcount++;

	/* get some relevant info here */

	theInst->inputsr = inSR;
	theInst->inputchans = inNCHANS;

	theInst->sfile_on = 1;

	/* offset is measured from location that is set up in rtinput() */
	theInst->fileOffset = rtInitialOffset
		+ ((int)(start * inSR)) * theInst->inputchans * sizeof(short);

	if (start >= inputFileTable[theInst->fdIndex].dur)
		fprintf(stderr, "\nWARNING: Attempt to read past end of input file: %s\n\n",
						inputFileTable[theInst->fdIndex].filename);

	return(1);
}

