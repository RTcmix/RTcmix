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
RTcmix::setparams(float sr, int nchans, int bufsamps, bool recording, float *mm_inbuf, float *mm_outbuf)
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
		die("rtsetparams", "Sampling rate must be greater than 0.");
		return -1;
	}
	
	if (NCHANS > MAXBUS) {
		die("rtsetparams", "You can only have up to %d output channels.", MAXBUS - 1);
		return -1;
	}
	
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
		SR = srate;
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
		die("rtsetparams", "You can only call rtsetparams once!");
		return -1;
	}
	if (n_args > 3 && pp[3] != 0.0) {
		// We used to support a 4th argument for output port (SGI only).  Could we make use of this for other platforms?
	}
	
	int bufsamps = n_args > 2 ? (int) p[2] : (int) Option::bufferFrames();
	bool recording = Option::record();
	
	return (double) setparams(p[0], (int)p[1], bufsamps, recording, NULL, NULL);
}

