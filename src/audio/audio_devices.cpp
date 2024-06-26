/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// audio_devices.cpp
// C++ functions for creating and configuring AudioDevice instances for input
// and output.
//

#include <string.h>
#include <RTcmix.h>
#include <sndlibsupport.h>
#include <ugens.h>
#include <prototypes.h>
#include <assert.h>
#include <RTOption.h>

#include "AudioDevice.h"
#include "AudioFileDevice.h"
#include "AudioIODevice.h"
#include "AudioOutputGroupDevice.h"
#include "DualOutputAudioDevice.h"
#include "audio_devices.h"

#define DEBUG	0

#ifdef NETAUDIO
char globalNetworkPath[128];			// Set by Minc/setnetplay.c
#endif

#ifdef MAX_DSP_AUDIO
// TODO PUT GLOBAL SPEC HERE
#endif

// Return pointers to the most recently specified audio device strings.
// "indevice" always overrides "device", and same with "outdevice".

#ifdef UNUSED
static const char *get_audio_device_name()
{
	if (strlen(RTOption::inDevice()) || strlen(RTOption::outDevice()))
		return NULL;
	if (strlen(RTOption::device()))
		return RTOption::device();
	return NULL;
}
#endif

static const char *get_audio_indevice_name()
{
	if (strlen(RTOption::inDevice()))
		return RTOption::inDevice();
	else if (strlen(RTOption::device()))
		return RTOption::device();
	return NULL;
}

static const char *get_audio_outdevice_name(int devIndex)
{
	if (strlen(RTOption::outDevice(devIndex)))
		return RTOption::outDevice(devIndex);
	else if (devIndex == 0 && strlen(RTOption::device()))
		return RTOption::device();
	return NULL;
}


AudioDevice *
create_audio_devices(int record, int play, int chans, float *ioSrate, int *buffersize, int numBuffers)
{
	int status;
	const char *inDeviceName = get_audio_indevice_name();
	const char *outDeviceName = get_audio_outdevice_name(0);
	AudioDevice *device = NULL;

#ifdef NETAUDIO
	// For backwards compatibility, we check to see if a network path was set
	// via the command line and stored in the global.
	if (strlen(globalNetworkPath) > 0)
		outDeviceName = globalNetworkPath;
#endif	// NETAUDIO

	// See audio_dev_creator.cpp for this function.
	device = createAudioDevice(inDeviceName, outDeviceName, record, play);
	if (device == NULL) {
		die("rtsetparams", "Failed to create audio device.");
		return NULL;
	}

	// We hand the device noninterleaved, full-range floating point buffers.
	int audioFormat = NATIVE_FLOAT_FMT | MUS_NON_INTERLEAVED;
	device->setFrameFormat(audioFormat, chans);
	
	double muteThreshold = RTOption::muteThreshold();
	if (muteThreshold > 0.0)
		device->setMuteThreshold(muteThreshold);

	int openMode = (record && play) ? AudioDevice::RecordPlayback
				   : (record) ? AudioDevice::Record
				   : AudioDevice::Playback;
	if (RTOption::checkPeaks())
		openMode |= AudioDevice::CheckPeaks;
	if (RTOption::reportClipping())
		openMode |= AudioDevice::ReportClipping;

#if DEBUG > 0
	printf("DEBUG: audio device: peak check: %d report clip: %d\n",
		   !!(openMode & AudioDevice::CheckPeaks),
		   !!(openMode & AudioDevice::ReportClipping));
#endif

	float srate = *ioSrate;
	
	if ((status = device->open(openMode, audioFormat, chans, srate)) == 0) {
		float newSrate = (float) device->getSamplingRate();
		// We check whether this reset is allowed at the next level up.
		*ioSrate = newSrate;
		int reqsize = *buffersize;
		int reqcount = numBuffers;
		if ((status = device->setQueueSize(&reqsize, &reqcount)) < 0) {
			die("rtsetparams", "%s", device->getLastError());
			delete device;
			return NULL;
		}
		int newSize = reqsize * numBuffers / reqcount;
		if (newSize != *buffersize) {
			rtcmix_advise("rtsetparams",
				   "Buffer size reset by audio device from %d to %d frames.",
					*buffersize, newSize);
			*buffersize = newSize;
		}
	}
	else
	{
		die("rtsetparams", "%s", device->getLastError());
		delete device;
		return NULL;
	}

	return device;
}

AudioDevice *
create_audio_file_device(AudioDevice *inDevice,
						 const char *outfilename,
						 int header_type,
						 int sample_format,
						 int chans,
						 float srate,
						 int normalize_output_floats,
						 int check_peaks)
{
	assert(rtsetparams_was_called());
	
	AudioDevice *device = NULL;

	AudioFileDevice *fileDevice = NULL;

    try {
        fileDevice = new AudioFileDevice(outfilename,header_type);
    }
	catch(...) {
		rterror("rtoutput", "Failed to create audio file device");
		return NULL;
	}
	
	// Here is the logic for opening the file device.  We do this all in
	// advance now, rather than over and over during rtwritesamps().

	const bool recording = RTOption::record();
	const bool playing = RTOption::play();
	const bool fileIsRawFloats = IS_FLOAT_FORMAT(sample_format) && !normalize_output_floats;
	
	int openMode = AudioFileDevice::Playback;
	if (playing | recording)
		openMode |= AudioDevice::Passive;		// Don't run thread for file device.
	if (!fileIsRawFloats || (check_peaks && !playing))
		openMode |= AudioDevice::CheckPeaks;	// Dont check peaks if HW already doing so.
	if (RTOption::reportClipping() && !playing)
		openMode |= AudioDevice::ReportClipping;	// Ditto for reporting of clipping

#if DEBUG > 0
	printf("DEBUG: file device: peak check: %d report clip: %d\n",
		   openMode & AudioDevice::CheckPeaks, openMode & AudioDevice::ReportClipping);
#endif

	// We send the device noninterleaved, floating point buffers.  If we are playing
	// to HW at the same time, the data received by the file device will already be
	// clipped.
	
	int audioFormat = NATIVE_FLOAT_FMT | MUS_NON_INTERLEAVED;
	if (playing) audioFormat |= MUS_CLIPPED;
	
	fileDevice->setFrameFormat(audioFormat, chans);

	// File format is interleaved and may be normalized.
	sample_format |= MUS_INTERLEAVED;
	if (normalize_output_floats)
		sample_format |= MUS_NORMALIZED;
	int ret = fileDevice->open(openMode, sample_format, chans, srate);
	
	if (ret == -1) {
		rterror("rtoutput", "Can't create output for \"%s\": %s", 
				 outfilename, fileDevice->getLastError());
		delete fileDevice;
		return NULL;
	}
	// Cheating -- should hand in queue size as argument!
	int queueSize = RTcmix::bufsamps();
	int count = 1;
	ret = fileDevice->setQueueSize(&queueSize, &count);
	if (ret == -1) {
		rterror("rtoutput", "Failed to set queue size on file device:  %s", 
				 fileDevice->getLastError());
		delete fileDevice;
		return NULL;
	}

	if (!playing && !recording) {	// To file only.
		// If we are only writing to disk, we only have a single output device. 
		assert(inDevice == NULL);	// We should not have been passed anything.
		device = fileDevice;
	}
	else {							// To file, plus record and/or playback.
		assert(inDevice != NULL);
		if (playing && !recording) {		// Dual outputs to both HW and file.
#if DEBUG > 0
			printf("DEBUG: Group output device for file and HW playback\n");
#endif
			bool fileDoesLimiting = !fileIsRawFloats;
			device = new DualOutputAudioDevice(inDevice,
											  fileDevice,
											  fileDoesLimiting);
		}
		else if (recording && !playing) {	// Record from HW, write to file.
#if DEBUG > 0
			printf("DEBUG: Dual device for HW record, file playback\n");
#endif
			device = new AudioIODevice(inDevice, fileDevice, true);
		}
		else {	// HW Record and playback, plus write to file.
#if DEBUG > 0
			printf("DEBUG: Dual device for HW record/playback, file playback\n");
#endif
			bool fileDoesLimiting = !fileIsRawFloats;
			device = new DualOutputAudioDevice(inDevice,
											   fileDevice,
											   fileDoesLimiting);
		}
	}

	if (RTOption::print()) {
		 printf("Output file set for writing:\n");
		 printf("      name:  %s\n", outfilename);
		 printf("      type:  %s\n", mus_header_type_name(header_type));
		 printf("    format:  %s\n", mus_data_format_name(MUS_GET_FORMAT(sample_format)));
		 printf("     srate:  %g\n", srate);
		 printf("     chans:  %d\n", chans);
	}

	return device;
}

