#ifndef _RTDEFS_H_
#define _RTDEFS_H_ 1

#define MAXCHANS 8

#define MAX_INPUT_FDS        128   /* number of slots in inputFileTable */

#define NO_DEVICE_FDINDEX    -1    /* value for inst fdIndex if unused */
#define NO_FD                -1    /* this InputDesc not in use */

/* SGI audio lib doesn't give us a file descriptor for an audio device,
   so we use this fake one in the InputDesc.
*/
#define AUDIO_DEVICE_FD      -2    /* not -1 ! */

/* definition of input file desc struct used by rtinput */
typedef struct inputdesc {
   char     *filename;      /* allocated by rtinput() */
   int      fd;             /* file descriptor, or NO_FD, or AUDIO_DEVICE */
   int      refcount;
   short    is_audio_dev;   /* true if input from audio device, not from file */
   short    header_type;    /* e.g., AIFF_sound_file (in sndlib.h) */
   short    data_format;    /* e.g., snd_16_linear (in sndlib.h) */
   int      data_location;  /* offset of sound data start in file */
   float    srate;
   short    chans;
   double   dur;
} InputDesc;


/* for insts - so they don't have to include globals.h */
extern int MAXBUF;
extern int NCHANS;
extern int RTBUFSAMPS;
extern float SR;

#endif /* _RTDEFS_H_ */
