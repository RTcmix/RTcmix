/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// audio_devices.cpp
// C++ functions for creating and configuring AudioDevice instances for input
// and output.
//

#include <string.h>
#include <globals.h>
#include <sndlibsupport.h>
#include <ugens.h>
#include <prototypes.h>
#include <assert.h>

#include "AudioDevice.h"
#include "AudioFileDevice.h"
#include "AudioIODevice.h"
#include "AudioOutputGroupDevice.h"
#include "audio_devices.h"

#ifdef NETAUDIO
char globalNetworkPath[128];			// Set by Minc/setnetplay.c
#endif

AudioDevice *globalAudioDevice;			// Used by Minc/intraverse.C
AudioDevice *globalOutputFileDevice;	// Used by rtsendsamps.C

static const int numBuffers = 2;		// number of audio buffers to queue up

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

	assert(rtsetparams_called);
	
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
	if (play_audio | record_audio)
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
	int queueSize = RTBUFSAMPS;
	int count = 1;
	ret = fileDevice->setQueueSize(&queueSize, &count);
	if (ret == -1) {
		rterror("rtoutput", "Failed to set queue size on file device:  %s", 
				 fileDevice->getLastError());
		return -1;
	}

	// This will soon go away entirely.
	globalOutputFileDevice = fileDevice;

	if (!play_audio && !record_audio) {				// To file only.
		// If we are only writing to disk, we only have a single output device. 
		globalAudioDevice = fileDevice;
	}
	else {	// To file, plus record and/or playback.
		if (play_audio && !record_audio) {	// Dual outputs to both HW and file.
			printf("DEBUG: Dual device for file and HW playback\n");
			globalAudioDevice = new AudioOutputGroupDevice(globalAudioDevice,
														   fileDevice);
			globalOutputFileDevice = NULL;
		}
		else if (record_audio && !play_audio) {	// Record from HW, write to file.
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

	if (print_is_on) {
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

static bool destroying = false;	// avoid reentrancy

void
destroy_audio_devices()
{
	if (!destroying) {
		destroying = true;
		globalAudioDevice->close();
	
		if (globalOutputFileDevice == globalAudioDevice) {
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
