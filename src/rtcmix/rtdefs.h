/* To avoid recursion in certain includes */
#ifndef _RTDEFS_H_
#define _RTDEFS_H_ 1

#define MAXCHANS 4
#define MAX_INPUT_FDS 128
#define AUDIO_DEVICE 9999999

/* definition of input file desc struct used by rtinput */
typedef struct inputdesc {
	char filename[1024];
	int fd;
	int refcount;
#ifdef USE_SNDLIB
	short header_type;        /* e.g., AIFF_sound_file (in sndlib.h) */
	short data_format;        /* e.g., snd_16_linear (in sndlib.h) */
	short chans;
	float srate;
#else
  #ifdef sgi
	void *handle;
  #endif
#endif
	int data_location;        /* offset of sound data start in file */
	float dur;
} InputDesc;

/* for insts - so they don't have to include globals.h */
extern int MAXBUF;
extern int NCHANS;
extern int RTBUFSAMPS;
extern float SR;

#endif /* _RTDEFS_H_ */

