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
#include <Option.h>

#include "AudioDevice.h"
#include "AudioFileDevice.h"
#include "AudioIODevice.h"
#include "audio_devices.h"

#ifdef NETAUDIO
char globalNetworkPath[128];			// Set by Minc/setnetplay.c
#endif

AudioDevice *globalAudioDevice;			// Used by Minc/intraverse.C
AudioDevice *globalOutputFileDevice;	// Used by rtsendsamps.C

static const int numBuffers = 2;		// number of audio buffers to queue up


// Return pointers to the most recently specified audio device strings.

const char *get_audio_device_name()
{
	if (strlen(Option::inDevice()) || strlen(Option::outDevice()))
		return NULL;
	if (strlen(Option::device()))
		return Option::device();
	return NULL;
}

const char *get_audio_indevice_name()
{
	if (strlen(Option::device()))
		return Option::device();
	if (strlen(Option::inDevice()))
		return Option::inDevice();
	return NULL;
}

const char *get_audio_outdevice_name()
{
	if (strlen(Option::device()))
		return Option::device();
	if (strlen(Option::outDevice()))
		return Option::outDevice();
	return NULL;
}


int
create_audio_devices(int record, int play, int chans, float srate, int *buffersize)
{
	int status;
	const char *inDeviceName = get_audio_indevice_name();
	const char *outDeviceName = get_audio_outdevice_name();
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
		return -1;
	}

	// We hand the device noninterleaved floating point buffers.
	int audioFormat = NATIVE_FLOAT_FMT | MUS_NON_INTERLEAVED;
	int openMode = (record && play) ? AudioDevice::RecordPlayback
				   : (record) ? AudioDevice::Record
				   : AudioDevice::Playback;
	device->setFrameFormat(audioFormat, chans);
	if ((status = device->open(openMode, audioFormat, chans, srate)) == 0)
	{
		int reqsize = *buffersize;
		int reqcount = numBuffers;
		if ((status = device->setQueueSize(&reqsize, &reqcount)) < 0) {
			die("rtsetparams", "%s", device->getLastError());
			return -1;
		}
		if (reqsize != *buffersize) {
			advise("rtsetparams",
				   "RTBUFSAMPS reset by audio device from %d to %d.",
					*buffersize, reqsize);
			*buffersize = reqsize;
		}
	}
	else
	{
		die("rtsetparams", "%s", device->getLastError());
		return -1;
	}

	globalAudioDevice = device;

	return status;
}

int create_audio_file_device(const char *outfilename,
							 int header_type,
							 int sample_format,
							 int chans,
							 float srate,
							 int normalize_output_floats,
							 int check_peaks)
{

	assert(rtsetparams_was_called());
	
	int fileOptions = 0;
	if (check_peaks)
		fileOptions |= AudioFileDevice::CheckPeaks;

	AudioFileDevice *fileDevice = new AudioFileDevice(outfilename,
													  header_type,
													  fileOptions);
													
	if (fileDevice == NULL) {
		rterror("rtoutput", "Failed to create audio file device");
		return -1;
	}
	int openMode = AudioFileDevice::Playback;
	if (Option::play() | Option::record())
		openMode |= AudioDevice::Passive;	// Don't run thread for file device.

	// We send the device noninterleaved floating point buffers.
	int audioFormat = NATIVE_FLOAT_FMT | MUS_NON_INTERLEAVED;
	fileDevice->setFrameFormat(audioFormat, chans);

	// File format is interleaved and may be normalized.
	sample_format |= MUS_INTERLEAVED;
	if (normalize_output_floats)
		sample_format |= MUS_NORMALIZED;
	int ret = fileDevice->open(openMode, sample_format, chans, srate);
	
	if (ret == -1) {
		rterror("rtoutput", "Can't create output for \"%s\": %s", 
				 outfilename, fileDevice->getLastError());
		return -1;
	}
	// Cheating -- should hand in queue size as argument!
	int queueSize = RTcmix::bufsamps();
	int count = 1;
	ret = fileDevice->setQueueSize(&queueSize, &count);
	if (ret == -1) {
		rterror("rtoutput", "Failed to set queue size on file device:  %s", 
				 fileDevice->getLastError());
		return -1;
	}

	// This will soon go away entirely.
	globalOutputFileDevice = fileDevice;

	if (!Option::play() && !Option::record()) {				// To file only.
		// If we are only writing to disk, we only have a single output device. 
		globalAudioDevice = fileDevice;
	}
	else {	// To file, plus record and/or playback.
		if (Option::play() && !Option::record()) {	// Dual outputs to both HW and file.
			printf("DEBUG: Independent devices for file and HW playback\n");
			// For this one, we need to leave the two globals for now, until
			// I write a dual-output AudioDevice.
			// Passive start takes NULL callback and context.
			if (fileDevice->start(NULL, NULL) == -1) {
				rterror("rtoutput", "Can't start file device: %s", 
				 		fileDevice->getLastError());
				return -1;
			}
		}
		else if (Option::record() && !Option::play()) {	// Record from HW, write to file.
			assert(globalAudioDevice != NULL);
			printf("DEBUG: Dual device for HW record, file playback\n");
			globalAudioDevice = new AudioIODevice(globalAudioDevice, fileDevice, true);
			globalOutputFileDevice = NULL;
		}
		else {	// HW Record and playback, plus write to file.
			if (fileDevice->start(NULL, NULL) == -1) {
				rterror("rtoutput", "Can't start file device: %s", 
				 		fileDevice->getLastError());
				return -1;
			}
		}
	}

	if (Option::print()) {
		 printf("Output file set for writing:\n");
		 printf("      name:  %s\n", outfilename);
		 printf("      type:  %s\n", mus_header_type_name(header_type));
		 printf("    format:  %s\n", mus_data_format_name(MUS_GET_FORMAT(sample_format)));
		 printf("     srate:  %g\n", srate);
		 printf("     chans:  %d\n", chans);
	}

	return 0;
}

int audio_input_is_initialized()
{
	return globalAudioDevice != NULL;
}

void
stop_audio_devices()
{
	// Turns out we need to close, not just stop (close calls stop)
	globalAudioDevice->close();
}

static bool destroying = false;	// avoid reentrancy

void
destroy_audio_devices()
{
	if (!destroying) {
		destroying = true;
		globalAudioDevice->close();
	
		if (globalOutputFileDevice == NULL || globalOutputFileDevice == globalAudioDevice) {
			delete globalAudioDevice;
			globalOutputFileDevice = NULL;	// Dont delete elsewhere.
		}
		globalAudioDevice = NULL;
	}
}

int
destroy_audio_file_device()
{
	int result = 0;
	if (globalOutputFileDevice) {
    	result = globalOutputFileDevice->close();
	}
                                                                                                    
	if (globalOutputFileDevice != globalAudioDevice) {
    	delete globalOutputFileDevice;
	}
	globalOutputFileDevice = NULL;

	return result;
}
