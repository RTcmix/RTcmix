/* RTcmix  - Copyright (C) 2004  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>		// for int32_t, etc.
#include <unistd.h>
#include <RTcmix.h>
#include <rtcmix_types.h>
#include "prototypes.h"
#include "byte_routines.h"
#include <sndlibsupport.h>
#include "utils.h"
#include <ugens.h>		// for warn, die
#include <maxdispargs.h>


// Used to read in samples from disk to a buffer which can then be used
// for rtinput("MMBUF", "bufname") calls.  This one uses the RTcmix internal
// soundfile-reading capabilities, as opposed to the methods used in
// Max/MSP (for [buffer~] objects) and iRTcmix (uses iOS 'file' locating
// and reading operations).  This was developed for use in the OpenFrameworks
// ofRTcmix; called by OF_buffer_load_set() in main.cpp.  The code was
// grabbed from the 'maketable("soundfile", ...)' RTcmix function (table.cpp).
//
// Another way to do this would be to create an RTcmix instrument that
// reads from said 'maketable()', but that wouldn't generalize it
// for use by all instruments that can read samples via 'rtinput()'
//
// 	-- BGG, 6/2013



// we will convert the buffer to floats no matter what format the soundfile
float *
sound_sample_buf_read(char *fname, double insk, double dur, int *nframes, int *nchans)
{
	// these are passed in to open_sound_file() and will be filled
	int data_format, data_location, file_chans;
	long file_samps;
	double srate;

	int fd = open_sound_file("sound_sample_buf_read", fname, NULL,
                &data_format, &data_location, &srate, &file_chans, &file_samps);
	if (fd == -1)
		return NULL;

	if (srate != RTcmix::sr())
		rterror("sound_sample_buf_read", "The input file sampling rate is %g, but "
			  "the output rate is currently %g.", srate, RTcmix::sr());

	long file_frames = file_samps / file_chans;
	long insk_frames = (long)(insk * srate);
	long read_frames = (long)(dur * srate);
	long read_samps = read_frames * file_chans;
	if (read_frames + insk_frames > file_frames) {
		read_frames = file_frames - insk_frames;
		// some of the rterror()s should probably be rtcmix_warn()s, but for
		// OpenFrameworks the print level won't have been set
		rterror("sound_sample_buf_read", "The total duration you have requested to read soundfile %s is longer than the file\n"
		"resetting duration to %f\n", fname, (float)read_frames/srate);
	}

	if (read_frames < 0) {
		rterror("sound_sample_buf_read", "you are attempting to skip beyond the end of the file");
		return NULL;
	}

	*nframes = read_frames;
	*nchans = file_chans;

	float *block = new float[read_samps];
	if (block == NULL) {
		rterror("sound_sample_buf_read", "Not enough memory for sound sample buf");
		return NULL;
	}

	int bytes_per_samp = mus_data_format_to_bytes_per_sample(data_format);

	char *buf = new char[read_samps * bytes_per_samp];
	if (buf == NULL) {
		rterror("sound_sample_buf_read",
									"Not enough memory for temporary buffer.");
		return NULL;
	}

	off_t seek_to = data_location + (insk_frames * file_chans * bytes_per_samp);
	if (lseek(fd, seek_to, SEEK_SET) == -1) {
		rterror("sound_sample_buf_read", "File seek error: %s", strerror(errno));
		return NULL;
	}

#if MUS_LITTLE_ENDIAN
	bool byteswap = IS_BIG_ENDIAN_FORMAT(data_format);
#else
	bool byteswap = IS_LITTLE_ENDIAN_FORMAT(data_format);
#endif
	bool is_float = IS_FLOAT_FORMAT(data_format);
	bool is_24bit = IS_24BIT_FORMAT(data_format);

	long bytes_read = read(fd, buf, read_samps * bytes_per_samp);
	if (bytes_read == -1) {
		rterror("sound_sample_buf_read", "File read error: %s", strerror(errno));
		return NULL;
	}

	float *blockp = block;
	if (is_float) { // float soundfile
		float *bufp = (float *)buf;
		for (long i = 0; i < read_samps; i++) {
			if (byteswap) byte_reverse4(bufp);	// modify *bufp in place
			*blockp++ = (float) *bufp++;
		}
	}
	else if (is_24bit) { // 24-bit soundfile
		unsigned char *bufp = (unsigned char *)buf;
		for (long i = 0; i < read_samps; i++) {
			if (data_format == MUS_L24INT) { // data_format == MUS_L24INT
				int samp = (int) (((bufp[2] << 24)
								  + (bufp[1] << 16)
								  + (bufp[0] << 8)) >> 8);
				*blockp++ = (float) samp / (float) (1 << 8);
				bufp += 3;
			} else {	// data_format == MUS_B24INT
				int samp = (int) (((bufp[0] << 24)
								  + (bufp[1] << 16)
								  + (bufp[2] << 8)) >> 8);
				*blockp++ = (float) samp / (float) (1 << 8);
				bufp += 3;
			}
		}
	}
	else { // 16bit integer file
		short *bufp = (short *)buf;
		for (long i = 0; i < read_samps; i++) {
			if (byteswap) {
				int16_t samp = reverse_int2(bufp);
				*blockp++ = (float) samp;
				bufp++;
			}
			else {
				*blockp++ = (float) *bufp++;
			}
		}
	}

	delete [] buf;
	sndlib_close(fd, 0, 0, 0, 0);

	return block;
}

