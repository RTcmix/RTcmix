/* This is set up so that all vars are defined when this file is included
   by main.C (which defines the MAIN preprocessor symbol), and declared
   extern when included by all other files.
   For C++ files including this one, all vars are wrapped by extern "C".
                                                              -JGG, 2/8/00
*/
#ifndef _GLOBALS_H_ 
#define _GLOBALS_H_ 1

#include "version.h"
#include "buffers.h"
#include "bus.h"  // FIXME: just for MAXBUS

#ifdef MAIN
#define GLOBAL
#else
#define GLOBAL extern
#endif

#ifdef __cplusplus

#include "../rtstuff/heap/heap.h"
GLOBAL rtQueue rtQueue[MAXBUS+2];
GLOBAL heap rtHeap;  // DT:  main heap structure used to queue instruments
                     // formerly Qobject *rtqueue[];
extern "C" {
#endif /* __cplusplus */

#ifdef LINUX
GLOBAL int in_port[MAXBUS];    /* array, in case sound driver uses many devs */
GLOBAL int out_port[MAXBUS];
#endif
#ifdef SGI
#include <dmedia/audio.h>
GLOBAL ALport in_port;
GLOBAL ALport out_port;
#endif

/* Note: these 4 vars also extern in rtdefs.h, for use by insts */
GLOBAL int MAXBUF;    /* NOTE NOTE NOTE: MAXBUF is a constant in SGI version! */
GLOBAL int NCHANS;
GLOBAL int RTBUFSAMPS;
GLOBAL float SR;
GLOBAL int audioNCHANS;


/* -------------------------------------------------------------------------- */
GLOBAL int oldSched;
GLOBAL int noParse;
GLOBAL int audio_on;
GLOBAL int play_audio;
GLOBAL int noaudio;              /* to delay socket parsing */
GLOBAL int full_duplex;
GLOBAL int audio_config;
GLOBAL int rtInteractive;
GLOBAL int print_is_on;
GLOBAL int rtsetparams_called;

/* for more than 1 socket, set by -s flag to CMIX as offset from MYPORT */
GLOBAL int socknew;

/* used in intraverse.C, traverse.C and rtsendsamps.c */
GLOBAL unsigned long bufStartSamp;

#include <pthread.h>
#ifdef MAIN      /* Have to do this because must be inited in definition. */
pthread_mutex_t heapLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pfieldLock = PTHREAD_MUTEX_INITIALIZER;
/* pthread_mutex_t heapLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; */
#else
GLOBAL pthread_mutex_t heapLock;
GLOBAL pthread_mutex_t pfieldLock;
#endif

/* -------------------------------------------------------------------------- */

#include "rtdefs.h"
GLOBAL InputDesc inputFileTable[MAX_INPUT_FDS];

GLOBAL BufPtr audioin_buffer[MAXBUS];    /* input from ADC, not file */
GLOBAL BufPtr aux_buffer[MAXBUS];
GLOBAL BufPtr out_buffer[MAXBUS];

GLOBAL int rtfileit;
GLOBAL int rtoutfile;

/* This should probably go someplace else in this file? */
typedef enum {
  NO = 0,
  YES
} Bool;

typedef enum {
  TO_AUX,
  AUX_TO_AUX,
  TO_OUT,
  UNKNOWN
} IBusClass;

GLOBAL short AuxPlayList[MAXBUS]; /* The playback order for AUX buses */

/* -------------------------------------------------------------------------- */
/* rtupdate stuff */

#define MAXPUPARR 100
#define MAXPUPS 20
#define NOPUPDATE 78787878 // hopefully never a real p-value!

GLOBAL int curtag;                /* current note tag */
GLOBAL int tags_on;               /* using note tags for rtupdates */
GLOBAL int tag_sem;

/* contains the values to be updated -- a recirculating array */
GLOBAL float pupdatevals[MAXPUPARR][MAXPUPS];


#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _GLOBALS_H_ */
