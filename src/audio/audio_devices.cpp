/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
// audio_devices.cpp
// C++ functions for creating and configuring AudioDevice instances for input
// and output.
//

#include <globals.h>
#include <sndlibsupport.h>
#include <ugens.h>
#include <prototypes.h>

#include "AudioDevice.h"
#include "AudioFileDevice.h"
#include "audio_devices.h"

AudioDevice *globalAudioDevice;		// Used by Minc/audioLoop.C
AudioDevice *globalOutputFileDevice;

static const int numBuffers = 2;		// number of audio buffers to queue up

int
create_audio_devices(int recordAndPlay, int chans, float srate, int *buffersize)
{
	int status;
	const char *inDeviceName = get_audio_indevice_name();
	const char *outDeviceName = get_audio_outdevice_name();
	AudioDevice *device = createAudioDevice(inDeviceName, outDeviceName, recordAndPlay != 0);
	// We hand the device noninterleaved floating point buffers.
	int audioFormat = NATIVE_FLOAT_FMT | MUS_NON_INTERLEAVED;
	int openMode = (recordAndPlay) ?
						AudioDevice::RecordPlayback : AudioDevice::Playback;
	device->setFrameFormat(audioFormat, chans);
	if ((status = device->open(openMode,
	  						   audioFormat,
							   chans,
							   srate)) == 0)
	{
		int reqsize = *buffersize;
		int reqcount = numBuffers;
		if ((status = device->setQueueSize(&reqsize, &reqcount)) < 0) {
			die("rtsetparams",
				"Trouble setting audio device queue size: %s",
				device->getLastError());
			return -1;
		}
		if (reqsize != *buffersize) {
			advise("rtsetparams",
					"RTBUFSAMPS reset by audio device from %d to %d",
					*buffersize, reqsize);
			*buffersize = reqsize;
		}
	}
	else
	{
		die("rtsetparams", "Trouble opening audio device: %s", 
			device->getLastError());
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
							 int check_peaks,
							 int play_audio_too)
{
	// Pass global options into the device.
	
	int fileOptions = 0;
	if (check_peaks)
		fileOptions |= AudioFileDevice::CheckPeaks;

	AudioFileDevice *fileDevice = new AudioFileDevice(outfilename,
													  header_type,
													  fileOptions);
													
	int openMode = AudioFileDevice::Playback;
	if (play_audio)
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
	if (play_audio) {
		// Passive start takes NULL callback and context.
		if (fileDevice->start(NULL, NULL) == -1) {
			rterror("rtoutput", "Can't start file device: %s", 
				 	fileDevice->getLastError());
			return -1;
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

	// If we are only writing to disk, we only have a single output device.  
	// When all this is finished, we should be able to "play" to file, with 
	// no distinction in the RTcmix code between to-disk and to-audio-hw.
	
	globalOutputFileDevice = fileDevice;
	if (!play_audio_too) {
		globalAudioDevice = fileDevice;
	}
	return 0;
}

int audio_input_is_initialized()
{
	return globalAudioDevice != NULL;
}

void
destroy_audio_devices()
{
	globalAudioDevice->close();
	
	if (globalOutputFileDevice == globalAudioDevice) {
		delete globalAudioDevice;
		globalOutputFileDevice = NULL;	// Dont delete elsewhere.
	}
	globalAudioDevice = NULL;
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
