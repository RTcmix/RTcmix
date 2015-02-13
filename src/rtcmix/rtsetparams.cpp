/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* Originally by Brad Garton and Doug Scott (SGI code) and Dave Topper
   (Linux code). Reworked for v2.3 by John Gibson.
   Reworked to use new AudioDevice code for v3.7 by Douglas Scott.
*/
#include <RTcmix.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include "audio_devices.h"
#include <ugens.h>
#include <Option.h>
#include "rtdefs.h"
#include "InputFile.h"

/* #define DEBUG */

int
RTcmix::setparams(float sr, int nchans, int bufsamps, bool recording, int bus_count)
{
	int         i;
	int         verbose = Option::print();
	int         play_audio = Option::play();
	int         record_audio = Option::record(recording);
	
	// FIXME: Need better names for NCHANS and RTBUFSAMPS. -JGG
	
	SR = sr;
	NCHANS = nchans;
	RTBUFSAMPS = bufsamps;
	
	int numBuffers = Option::bufferCount();
	
	if (SR <= 0.0) {
		return die("rtsetparams", "Sampling rate must be greater than 0.");
	}
	
	if (bus_count < NCHANS) {
		return die("rtsetparams", "Bus count must be >= channel count");
	}
	if (bus_count < MAXBUS && bus_count >= MINBUS) {
		busCount = bus_count;
	}
	else {
		return die("rtsetparams", "Bus count must be between %d and %d", MINBUS, MAXBUS);
	}

	if (NCHANS > busCount) {
		return die("rtsetparams", "You can only have up to %d output channels.", busCount);
	}
	
#if !defined(MAXMSP)
	// Now that much of our global state is dynamically sized, the initialization
	// of that state needs to be delayed until after we know the bus count as
	// passed via rtsetparams().  MAX requires that this all be done at bringup time
	// though because it is legal to parse a score before calling rtsetparams().
	
	init_globals();
#endif
	/* play_audio is true unless user has called set_option("audio_off") before
	 rtsetparams. This would let user run multiple jobs, as long as only one
	 needs the audio drivers.  -JGG
	 
	 record_audio is false unless user has called set_option("full_duplex_on")
	 or has explicity turned record on by itself via set_option("record_on"),
	 or has already called rtinput("AUDIO").  -DS
	 */
	if (play_audio || record_audio) {
		int nframes = RTBUFSAMPS;
		float srate = SR;
		rtcmix_debug(NULL, "RTcmix::setparams creating audio device(s)");
		if ((audioDevice = create_audio_devices(record_audio, play_audio,
												NCHANS, &srate, &nframes, numBuffers)) == NULL)
			return -1;

		/* These may have been reset by driver. */
		RTBUFSAMPS = nframes;
		if (srate != SR) {
			if (!Option::requireSampleRate()) {
				rtcmix_advise("rtsetparams",
							  "Sample rate reset by audio device from %f to %f.",
							  SR, srate);
				SR = srate;
			}
			else {
				return die("rtsetparams", "Sample rate could not be set to desired value.\nSet \"require_sample_rate=false\" to allow alternate rates.");
			}
		}
	}
	
	/* Allocate output buffers. Do this *after* opening audio devices,
	 in case OSS changes our buffer size, for example.
	 */
	for (i = 0; i < NCHANS; i++) {
		allocate_out_buffer(i, RTBUFSAMPS);
	}
	
#ifdef MULTI_THREAD
	InputFile::createConversionBuffers(RTBUFSAMPS);
#endif

	/* inTraverse waits for this. Set it even if play_audio is false! */
	pthread_mutex_lock(&audio_config_lock);
	audio_config = 1;
	pthread_mutex_unlock(&audio_config_lock);
	
#ifdef EMBEDDED
	if (verbose >= MMP_PRINTALL)
#endif
		rtcmix_advise(NULL, "Audio set:  %g sampling rate, %d channels\n", SR, NCHANS);
	
	
	rtsetparams_called = 1;	/* Put this at end to allow re-call due to error */
	
	return 0;
}

int RTcmix::resetparams(float sr, int chans, int bufsamps, bool recording)
{
	rtcmix_debug(NULL, "RTcmix::resetparams entered");
	if (sr != SR) {
		rtcmix_warn(NULL, "Resetting SR from %.2f to %.2f", SR, sr);
		SR = sr;
	}
	if (chans != NCHANS) {
		rtcmix_warn(NULL, "Resetting NCHANS from %d to %d", NCHANS, chans);
		NCHANS = chans;
	}
	if (bufsamps != RTBUFSAMPS) {
		rtcmix_warn(NULL, "Resetting RTBUFSAMPS from %d to %d", RTBUFSAMPS, bufsamps);
		RTBUFSAMPS = bufsamps;
	}
	
	// For now, we have to do this operation by itself because RTcmix::close() freed these buffers.
	// TODO: Make open/close create/destroy methods more intuitive.
	
	init_buf_ptrs();

	int	numBuffers = Option::bufferCount();
	int	play_audio = Option::play();
	int	record_audio = Option::record(recording);
	if (play_audio || record_audio) {
		int nframes = RTBUFSAMPS;
		float srate = SR;
		rtcmix_debug(NULL, "RTcmix::resetparams re-creating audio device(s)");
		if ((audioDevice = create_audio_devices(record_audio, play_audio,
												NCHANS, &srate, &nframes, numBuffers)) == NULL)
		{
			return -1;
		}
		/* These may have been reset by driver. */
		RTBUFSAMPS = nframes;
		SR = srate;
	}
	
	/* Reallocate output buffers. Do this *after* opening audio devices,
	 in case the device changes our buffer size, for example.
	 */
	for (int i = 0; i < NCHANS; i++) {
		allocate_out_buffer(i, RTBUFSAMPS);
	}
	/* Reallocate input buffers if we are in record mode.  This was initially done by rtinput() */
	if (record_audio) {
		for (int i = 0; i < NCHANS; i++) {
			allocate_audioin_buffer(i, RTBUFSAMPS);
		}
	}
#ifdef MULTI_THREAD
	InputFile::createConversionBuffers(RTBUFSAMPS);
#endif

	/* inTraverse waits for this. Set it even if play_audio is false! */
	pthread_mutex_lock(&audio_config_lock);
	audio_config = 1;
	pthread_mutex_unlock(&audio_config_lock);
	
	rtcmix_debug(NULL, "RTcmix::resetparams exited");
	return 0;
}

/* ---------------------------------------------------------- rtsetparams --- */
/* Minc function that sets output sampling rate (p0), maximum number of
   output channels (p1), and (optionally) I/O buffer size (p2).

   On SGI, the optional p3 lets you specify the output port as a string
   (e.g., "LINE", "DIGITAL", etc.).

   Based on this information, rtsetparams allocates a mono output buffer
   for each of the output channels, and opens output devices.
*/
double
RTcmix::rtsetparams(float p[], int n_args, double pp[])
{
#ifdef EMBEDDED
// BGG mm -- ignore this one, use RTcmix::setparams()
//    (called by RTcmix_setparams() in main.cpp)
	return 0.0;
#endif

	if (rtsetparams_was_called()) {
		return die("rtsetparams", "You can only call rtsetparams once!");
	}
		
	int bufsamps = (n_args > 2) ? (int) p[2] : (int) Option::bufferFrames();
	bool recording = Option::record();
	int numBusses = (n_args > 3) ? (int)p[3] : DEFAULT_MAXBUS;
	
	return (double) setparams(p[0], (int)p[1], bufsamps, recording, numBusses);
}

