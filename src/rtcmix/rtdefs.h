#ifndef _RTDEFS_H_
#define _RTDEFS_H_ 1

#ifndef MAXCHANS
#define MAXCHANS 8
#elif MAXCHANS < 8
#error MAXCHANS must be at least 8 (to match MINBUS)
#endif

#define RESERVE_INPUT_FDS    20  // subtract this from max number of input files

#define NO_DEVICE_FDINDEX    -1    /* value for inst fdIndex if unused */
#define NO_FD                -1    /* this InputFile not in use */


// BGG mm --------------
#define USE_MM_BUF            -777 // use [buffer~] sample data for input

#define MAX_INLETS				64	// For RTInletPField
// BGG mm end --------------


#define DONT_SCHEDULE			-1	/* returned by Instr->init() on fatal err */

/* SGI audio lib doesn't give us a file descriptor for an audio device,
   so we use this fake one in the InputFile.
*/
#define AUDIO_DEVICE_FD      -2    /* not -1 ! */

// InputDesc is now InputFile and is declared in InputFile.h

#endif /* _RTDEFS_H_ */
