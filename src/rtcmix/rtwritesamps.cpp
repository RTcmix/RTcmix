/* RTcmix  - Copyright (C) 2001  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.

   Apr 2004: Converted from C source to C++ source by DS -- see deleted .c file
   for change history.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>  /* for strerror */
#include <math.h>    /* for fabs */
#include <errno.h>
#include <globals.h>
#include <prototypes.h>
#include "AudioDevice.h"
#include "audio_devices.h"

/* --------------------------------------------------------- rtwritesamps --- */
/* Write the current non-interleaved output buffers to the fileDevice.  
   Currently the AudioDevice supports 16-bit and 24-bit signed
   integer and 32-bit floating-point files, in both byte orders.  Floats have
   a scaling option that forces the normal range of values to fall between 
   -1.0 and 1.0 (set at file device creation time).
*/

int
rtwritesamps(AudioDevice *fileDevice)
{
	const int nframes = RTBUFSAMPS;

   /* This catches our new case where rtoutput() failed but was ignored */
   if (rtfileit < 0) {
      fprintf(stderr, "rtwritesamps: No output file open (rtoutput failed).\n");
      exit(1);
   }
   
   int framesWritten = fileDevice->sendFrames(out_buffer, nframes);
   
   if (framesWritten != nframes) {
      fprintf(stderr, "rtwritesamps error: %s\n", fileDevice->getLastError());
      exit(1);
   }

   return 0;
}

/* ----------------------------------------------------------- rtcloseout --- */
/* Close the output file device. */
int
rtcloseout()
{
	int result = 0;

	if (rtfileit != 1)          /* nothing to close */
		return 0;

	result = destroy_audio_file_device();

	if (result == -1) {
		fprintf(stderr, "rtcloseout: Error closing \"%s\" (%s)\n",
				rtoutsfname, strerror(errno));
		return -1;
	}

	return 0;
}


