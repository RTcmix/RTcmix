/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
/* rev'd for v2.3, JGG, 20-Feb-00 */
/* rev'd for v3.7 (converted to C++ source), DS, April-04 */

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <globals.h>
#include <prototypes.h>
#include <buffers.h>
#include "../rtstuff/rtdefs.h"
#include "AudioDevice.h"

extern AudioDevice *globalInputDevice;	// from audio_devices.cpp

/* local prototypes */

static int read_from_audio_device(void);


/* ----------------------------------------------- read_from_audio_device --- */
static int
read_from_audio_device(AudioDevice *inputDevice)
{
	int result;

	result = inputDevice->getFrames(audioin_buffer, RTBUFSAMPS);
	if (result == -1) {
		fprintf(stderr, "read_from_audio_device got error: %s\n",
				inputDevice->getLastError());
		return -1;
	}

   return 0;
}


/* ----------------------------------------------------------- rtgetsamps --- */
void
rtgetsamps(AudioDevice *inputDevice)
{
   int   err;

   assert(audio_on == 1);

	// This is a hack:  until I can create an AudioDevice which is guaranteed
	// to work in full duplex, we need to override the device handed to us to
	// point to the global input device.  Otherwise, we will be calling read on
	// a device possibly configured only for output (playback).

	if (!globalInputDevice || globalInputDevice == inputDevice)
	   err = read_from_audio_device(inputDevice);
	else
	   err = read_from_audio_device(globalInputDevice);

   if (err)
      fprintf(stderr, "rtgetsamps: bad read from audio device\n");
}


