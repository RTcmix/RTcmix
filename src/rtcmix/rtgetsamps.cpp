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

static int dev_ibuf_allocated = 0;
extern AudioDevice *globalInputDevice;	// from audio_devices.cpp

/* Array of input buffers for audio device, in format wanted by driver.
   If driver wants interleaved samps, use only dev_ibuf[0], which contains
   interleaved samples. Otherwise, each buffer in dev_ibuf contains one
   channel of samples.
*/
static IBufPtr dev_ibuf[MAXBUS];

/* local prototypes */
static void allocate_dev_ibuf(void);
static int read_from_audio_device(void);



/* ---------------------------------------------------- allocate_dev_ibuf --- */
/* Allocate either one interleaved buffer or multiple single buffers,
   depending on whether MONO_DEVICES is defined.
*/
static void
allocate_dev_ibuf()
{
#ifdef MONO_DEVICES
   int i;

   for (i = 0; i < audioNCHANS; i++)
      dev_ibuf[i] = allocate_ibuf_ptr(RTBUFSAMPS);
#else /* !MONO_DEVICES */
   dev_ibuf[0] = allocate_ibuf_ptr(RTBUFSAMPS * NCHANS);
#endif /* !MONO_DEVICES */
}


/* ----------------------------------------------- read_from_audio_device --- */
static int
read_from_audio_device(AudioDevice *inputDevice)
{
	int result;

	IBufPtr src = dev_ibuf[0];

	result = inputDevice->getFrames(src, RTBUFSAMPS);
	if (result == -1) {
		fprintf(stderr, "read_from_audio_device got error: %s\n",
				inputDevice->getLastError());
		return -1;
	}

	const int chans = audioNCHANS;	// avoid using globals in tight loops
	const int samps = RTBUFSAMPS;
	
	for (int n = 0; n < chans; n++) {
		BufPtr dest = audioin_buffer[n];
		int   i, j;
		for (i = 0, j = n; i < samps; i++, j += chans)
			dest[i] = (BUFTYPE) src[j];
	}

   return 0;
}


/* ----------------------------------------------------------- rtgetsamps --- */
void
rtgetsamps(AudioDevice *inputDevice)
{
   int   err;

   assert(audio_on == 1);

   if (!dev_ibuf_allocated) {
	   allocate_dev_ibuf();
	   dev_ibuf_allocated = 1;
   }
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


