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

enum {
    FUNCTION_NOT_FOUND      = 1,    /* error, but alternately treated as warning */
    NO_ERROR                = 0,
    DONT_SCHEDULE           = -1,	/* returned by Instr->init() on fatal err */
    PARAM_ERROR             = -2,   /* passed-in value or value reached in curve, etc., out of range */
    AUDIO_ERROR             = -3,   /* error with reading or writing audio to/from HW device */
    FILE_ERROR              = -4,   /* error seeking in, reading or writing to file */
    SYSTEM_ERROR            = -5,   /* unspecified fatal error */
    MEMORY_ERROR            = -6
};

/* SGI audio lib doesn't give us a file descriptor for an audio device,
   so we use this fake one in the InputFile.
*/
#define AUDIO_DEVICE_FD      -2    /* not -1 ! */

// InputDesc is now InputFile and is declared in InputFile.h

#endif /* _RTDEFS_H_ */
