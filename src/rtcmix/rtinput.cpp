#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../rtstuff/rtdefs.h"

#ifdef USE_SNDLIB
  #include <errno.h>
  #include "../H/sndlibsupport.h"
#else
  #include "../H/sfheader.h"
#endif

extern int print_is_on;
extern float SR;
extern int NCHANS;

/* This stuff not yet in LINUX */

#ifdef sgi
ALport in_port;
int audio_on = 0;
int audioNCHANS; /* this is for the reads on the audio device */
#endif

InputDesc inputFileTable[MAX_INPUT_FDS];
int rtInputIndex = -1;  /* current index into inputFileTable */
off_t rtInitialOffset;  /* current initial offset in file */

int input_on;
double inSR;
int inNCHANS;
char *rtsfname;
extern int audio_on;

/* this routine is used in the Minc score to open up a file for
* subsequent reads by RT Instruments.  "rtsetinput" is used in
* the init member functions of the Instruments to access the file.
*
* if p[0] is "AUDIO" and if [optional] p1 is "MIC", "LINE" or "DIGITAL"
* it will set up the input device to do real-time reading of sound
* [optional] p[2] is nchans for input (default is stereo)
*
* p[0] = input soundfile name; p[1] = "MIC", "LINE", or "DIGITAL" (if
* p[0] == "AUDIO"); p[2] = (optional) input chans for real-time audio
*
*/

double rtinput(float *p, short n_args, double *pp)
{
#ifdef sgi  // but note that sgi code in this file is way out of date
	AFfilehandle rtinfile;
	ALconfig in_port_config;
#endif
	struct stat sfst;
#ifndef USE_SNDLIB
	SFHEADER sfh;
#endif
	int n, fd, result, header_type, data_format, data_location, nsamps;
	int tint;	

	/* Here is a fine example of cmix's ancient legacy. */
	/* Since we're casting stuff into doubles, which */
	/* are sometimes pointers to char's ... we need to */
	/* be sure that "bringing them back down" works */
	/* tint used to be an int ... which is to say that if */
	/* the double represented by pp[0] was too big, due to */
	/* variations in memory addressing by compilers, etc... */
	/* you'd be screwed.  So we make it into a long, to */
	/* provide a higher degree of "safety." */

	/* Thanks to Brad Garton for explaining that to me. */
	/* DT 2/10/97 */

        tint = (int)pp[0];
        rtsfname = (char *)tint;
   
	/* Input from soundfile / not audio device ... yet */
	input_on = 0;	

	if (strcmp(rtsfname, "AUDIO") == 0) {
	    // printf("rtinput():  setting up audio input\n");

	/* input_on toggles rtsetinput to take data from file or audio device
	 * With this set, the instrument does not have to worry about a file
	 * descriptor index.
	 */
	    input_on = 1;
	    inSR = SR;
	    inNCHANS = NCHANS;

	/* audio_on signals traverse() to grab buffers from the audio device */

	    audio_on = 1;
	    return 1.0;
	}


	/* input from soundfile -- not audio device */

	/* See if this file has already been opened. */

	for (n = 0; n < MAX_INPUT_FDS; n++) {
		if (!strcmp(inputFileTable[n].filename, rtsfname)) {
			rtInputIndex = n;
			return 0.0;
		}
	}

	/* It's a new file name, so open it and read its header. */

#ifdef USE_SNDLIB

	/* See if file exists and is a regular file or link. */
	if (stat(rtsfname, &sfst) == -1) {
		fprintf(stderr, "%s: %s\n", rtsfname, strerror(errno));
		return -1.0;
	}
	if (!S_ISREG(sfst.st_mode) && !S_ISLNK(sfst.st_mode)) {
		fprintf(stderr, "%s is not a regular file or a link.\n", rtsfname);
		return -1.0;
	}

	/* Open the file and read its header. Then info available from
	   sndlib query functions. We use the new API in sndlibsupport.c
	   instead of the cmix legacy support, because we'll also use
	   sndlib for file I/O in rtwritesamps.C and rtgetin.C. The legacy
	   functions don't bother to set up sndlib I/O.
	*/
	fd = sndlib_open_read(rtsfname);
	if (fd == -1) {
		fprintf(stderr, "Can't read header of \"%s\" (%s)\n",
												rtsfname, strerror(errno));
		return -1.0;
	}

	header_type = c_snd_header_type();

	if (NOT_A_SOUND_FILE(header_type)) {
		fprintf(stderr, "\"%s\" is probably not a sound file\n", rtsfname);
		sndlib_close(fd, 0, 0, 0, 0);
		return -1.0;
	}

	data_format = c_snd_header_format();

	if (INVALID_DATA_FORMAT(data_format)) {
		fprintf(stderr, "\"%s\" has invalid sound data format\n", rtsfname);
		sndlib_close(fd, 0, 0, 0, 0);
		return -1.0;
	}

	data_location = c_snd_header_data_location();
	nsamps = c_snd_header_data_size();          /* samples, not frames */

	inSR = (double)c_snd_header_srate();
	inNCHANS = c_snd_header_chans();

#else /* !USE_SNDLIB */

	/* using the built-in cmix macros in sfheader.h */
	readopensf(rtsfname,fd,sfh,sfst,"head",result);
	if (result == -1)
		return -1.0;       /* error reported in readopensf */

	inSR = sfsrate(&sfh);
	inNCHANS = sfchans(&sfh);

	data_location = getheadersize(&sfh);
	nsamps = (sfst.st_size - data_location) / sfclass(&sfh);

#endif /* !USE_SNDLIB */

	if (inSR != SR) {
		fprintf(stderr, "WARNING: the input file sampling rate is %f; but the output rate is currently %f\n",inSR, SR);
	}

	/* store initial offset as global for use by rtsetinput() */
	rtInitialOffset = data_location;

	if (print_is_on) {
	  printf("Input file %s set for reading\n", rtsfname);
#ifdef USE_SNDLIB
	  printf("    type:  %s\n", sound_type_name(header_type));
	  printf("  format:  %s\n\n", sound_format_name(data_format));
#endif
	  printf("   SR: %f   nchannels: %d\n", inSR, inNCHANS);
	}

	/* New file:  take file descriptor and put it into the first
	 * available slot in the inputFileTable.  rtInputIndex is the value
	 * that will be used by any instrument created after this call to
	 * rtinput().  Also copy the filename for later checking, and fill
	 * in the other fields in the InputDesc struct (see rtdefs.h).
	 */
	for (n = 0; n < MAX_INPUT_FDS; n++) {
	    if (inputFileTable[n].fd == 0) {
		rtInputIndex = n;
		strcpy(inputFileTable[n].filename, rtsfname);
		inputFileTable[n].fd = fd;
		inputFileTable[n].refcount = 0;
#ifdef USE_SNDLIB
		inputFileTable[n].header_type = header_type;
		inputFileTable[n].data_format = data_format;
#endif
		inputFileTable[n].data_location = data_location;
		inputFileTable[n].dur = (float)(nsamps / inNCHANS) / inSR;
		break;
	    }
	}

	/* if this is true we have used up all input descriptors in our array */
	if (n == MAX_INPUT_FDS) {
	    fprintf(stderr, "You have exceeded the input file limit (%d)!\n",
		    MAX_INPUT_FDS);
	    return -1.0;
	}

	return 0.0;
}

