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

extern AudioDevice *globalAudioDevice;	// audio_devices.cpp

/* ----------------------------------------------------------- rtgetsamps --- */
void
rtgetsamps(AudioDevice *inputDevice)
{
	assert(audio_on == 1);
	// Sorrowful hack:  The dual AudioDevice run the callback on the playback
	// device, which means it cannot pass the record device to this function.
	if (globalAudioDevice->getFrames(audioin_buffer, RTBUFSAMPS) < 0)
	{
		fprintf(stderr, "rtgetsamps error: %s\n", globalAudioDevice->getLastError());
	}
}


