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
#else
  #ifdef sgi
	void *handle;
  #endif
#endif
	int data_location;        /* offset of sound data start in file */
	float dur;
} InputDesc;

extern float *outbuff;
extern float *outbptr;
extern short *inbuff;  // DT:  for use with real-time audio input

// NOTE: MAXBUF is a constant in SGI version!
extern int MAXBUF;

// NOTE: these are wrapped with extern "C" in SGI version
extern float SR;
extern int NCHANS;
extern int RTBUFSAMPS;

#endif  /* _RTDEFS_H_ */
