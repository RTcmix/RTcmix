/* This file contains prototypes for functions used by the RTcmix core,
   not by instruments or utility programs.   -JGG
*/
#ifndef _PROTOTYPES_H_ 
#define PROTOTYPES_H_ 1

#include <buffers.h>    /* for BufPtr */
#include <rtcmix_types.h>

#ifdef __cplusplus

// These functions are declared as C++ functions.

/* rtsendsamps.C */
class AudioDevice;
void rtsendzeros(AudioDevice *, int also_write_to_file);
void rtsendsamps(AudioDevice *);
void rtreportstats(AudioDevice *);

/* rtgetsamps.C */
void rtgetsamps(AudioDevice *);

/* rtwritesamps.C */
int rtwritesamps(AudioDevice *);

/* dispatch.cpp */
#ifdef PFIELD_CLASS
int dispatch(const char *str, const Arg args[], int n_args, Arg *retarg);
#endif

double dispatch(const char *str, double *pp, int n_args, void **inst);

#ifdef PFIELD_CLASS
/* addcheckfuncs.cpp */
int checkfunc(const char *funcname, const Arg arglist[], const int nargs,
   Arg *retval);
#endif /* PFIELD_CLASS */

/* Note that C++ functions prototyped below really are defined within
   extern "C" braces in their files.
*/
extern "C" {

#endif /* __cplusplus */

/* audio_port.c:  prototypes in audio_port.h */

/* buffers.c:  prototypes in buffers.h */

/* checkfuncs.c */
double checkfuncs(const char *fname, double *pp, int n_args);

/* checkInsts.C */
#ifdef PFIELD_CLASS
double checkInsts(const char *instname, const Arg arglist[], const int nargs,
   Arg *retval);
#else
double checkInsts(const char *fname, double *pp, int n_args, void **inst);
#endif

/* this is just a wrapper for C files to call */
double parse_dispatch(const char *str, double *pp, int n_args, void **inst);


/* audioLoop.C */
int runMainLoop(void);
int waitForMainLoop(void);

/* parseit.C */
void *parseit(void *);

/* rtdispatch.C */
double rtdispatch(char *fname, double *pp, int n_args, void **inst);

/* rtgetin.C */
int read_samps(int fd, int data_format, int file_chans, BufPtr dest,
                                    int dest_chans, int dest_frames);

/* rtinput.c */
int get_last_input_index(void);
int open_sound_file(char *sfname, int *header_type, int *data_format,
					int *data_location, double *srate, int *nchans, long *nsamps);

/* rtsetparams.C */
double rtsetparams(float *p, int n_args, double *pp);
void close_audio_ports(void);

/* rtwritesamps.C */
int rtcloseout(void);

/* set_option.c */
const char *get_audio_device_name(void);
const char *get_audio_indevice_name(void);
const char *get_audio_outdevice_name(void);
int set_option(float *p, int n_args, double *pp);

/* sockit.C */
void *sockit(void *);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _PROTOTYPES_H_ */
