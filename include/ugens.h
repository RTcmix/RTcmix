/*
 *	ugens.h: define unit generator structure
 */
#ifndef _UGENS_H_ 
#define _UGENS_H_ 1

#define	UG_NSIZ	7	/*  Max len of UG name	*/
#define NAMESIZE 128    /* Max size of file name */
#define	UG_NULL	(struct ug_item *)0

#include <sys/types.h>
#include <rt_types.h>

#define NFILES       12       /* maximum number of opened files */

struct	ug_item	{
	struct	ug_item	*ug_next;
	double 	(*ug_ptr)();	/*  Pointer to the function	*/
	char	*ug_name;
};

typedef	struct ug_item	ug_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void merror(const char *);

/* times per second to update control-rate variables;
   defined in sys/minc_functions.c
*/
extern int resetval;
 
/*  structure to pass to gen routines */
struct gen {
           int size;           /* size of array to load up */
           int nargs;          /* number of arguments passed in p array */
           float *pvals;       /* address of array of p values */
           double *array;       /* address of array to be loaded up */
           int slot;	       /* slot number, for fnscl test */
           };


#ifndef SOUND
extern int (*addoutpointer[NFILES])();
extern int (*layoutpointer[NFILES])();
extern int (*wipeoutpointer[NFILES])();
extern int (*getinpointer[NFILES])();
extern int (*bwipeoutpointer[NFILES])();

#define ADDOUT(x,y)  (*addoutpointer[y])(x,y)
#define LAYOUT(x,l,y)  (*layoutpointer[y])(x,l,y)
#define WIPEOUT(x,y) (*wipeoutpointer[y])(x,y)
#define GETIN(x,y)   (*getinpointer[y])(x,y)
#define BWIPEOUT(x,y,z) (*bwipeoutpointer[y])(x,y,z)
#endif  /* SOUND */
#define  ABS(x) ((x < 0) ? (-x) : (x))
#define  SIGN(x) (ABS(x)/(x ? x : 1.))

/* declarations of units */

typedef enum {
   NO_INTERP = 0,
   LINEAR_INTERP
} InterpolationType;
double *resample_gen(double [], int, int, InterpolationType);

typedef enum {
   ADD_GENS,
   MULT_GENS
} GenModType;
int combine_gens(int, int, int, int, GenModType, char *);
int install_gen(int, int, double *);
double makegen(float [], int, double []);
double *floc(int);
int fsize(int);

extern int (*getsample)();
#define GETSAMPLE (*getsample)

float allpass(float, float *);
float allpole(float, int*, int, float*, float*);
double ampdb(float);
double dbamp(float);
float *ballpole(float*, long*, long, float*, float*, float*, long);
float *bbuzz(float, float, float, double*, float*, float*, long);
float boost(float);
int boscili(float, float, double*, int, float*, float*, int);
float bpluck(float, float*);
float breson(float*, float*, float*, int);
void sbrrand(unsigned int);
void brrand(float, float*, int);
float buzz(float, float, float, double*, float*);
float comb(float, float*);
void combset(float, float, float,int, float*);
double cpsmidi(double);
double cpsoct(double);
double cpspch(double);
float delget(float*, float, int*);
void delput(float, float*, int*);
void delset(float, float*, int*, float);
float dliget(float*, float, int*);
float evp(long, double*, double*, float*);
void evset(float, float, float, float, int, float*);
float hcomb(float,float,float*);
void hplset(float, float, float, float, float, float, int, float*);
float hpluck(float, float*);
double midicps(double);
double midioct(double);
double midipch(double);
double octcps(double);
double octmidi(double);
double octpch(double);
float oscil(float, float, double*, int, float*);
float oscili(float, float, double*, int, float*);
float osciln(float, float, double*, int, float*);
float oscilni(float, float, double*, int, float*);
double pchcps(double);
double pchmidi(double);
double pchoct(double);
double octlet(unsigned char *);
double cpslet(unsigned char *);
double pchlet(unsigned char *);
float pluck(float, float*);
void pluckset(float, float, float, float, float*, float);
float randf(float*, float);
float crandom(float);
float reson(float, float*);
void rsnset(float SR, float, float, float, float, float*);
void rszset(float SR, float, float, float, float*);
float resonz(float, float*);
void bresonz(float*, float*, float*, int);
float reverb(float, float*);
int setline(float [], short, int, double []);
void srrand(unsigned int);
void rvbset(float SR, float, int, float*);
#ifdef EMBEDDED
float rtcmix_table(long, double*, float*);
#else
float table(long, double*, float*);
#endif
float tablei(long, double*, float*);
void tableset(float SR, float, int, float*);
float wshape(float, double*, int);
float rrand(void);
int getsetnote(float start, float dur, int filenum);

#include "spray.h"
void sprayinit(struct slist *slist, int size, unsigned int seed);
int spray(struct slist *slist);

/* sound.c */
int setnote(float start, float dur, int fno);
int bgetin(float *input, int fno, int size);
void blayout(float *out, int *chlist, int fno, int size);
void baddout(float *out, int fno, int size);
void bwipeout(float *out, int fno, int size);
int endnote(int xno);
void _flushbuf(int fno);
void _chkpeak(int fno);
off_t _readit(int fno);
off_t _writeit(int fno);
void _backup(int fno);
void _forward(int fno);
void closesf(void);
void closesf_noexit(void);

/* minout.c */
off_t inrepos(int samps, int fno);
off_t outrepos(int samps, int fno);

/* fnscl.c */
void fnscl(struct gen *gen);

/* system error status values.  These are returned up through to the parser */

typedef enum {
    FUNCTION_NOT_FOUND      = 1,    /* error, but alternately treated as warning */
    NO_ERROR                = 0,
    DONT_SCHEDULE           = -1,	/* returned by Instr->init() on fatal err */
    PARAM_ERROR             = -2,   /* passed-in value or value reached in curve, etc., out of range */
    CONFIGURATION_ERROR     = -3,   /* instrument created before rtsetparams, etc. */
    AUDIO_ERROR             = -4,   /* error with reading or writing audio to/from HW device */
    FILE_ERROR              = -5,   /* error seeking in, reading or writing to file */
    SYSTEM_ERROR            = -6,   /* unspecified fatal error */
    RESOURCE_ERROR          = -7,   /* exceeded file limit, etc. */
    MEMORY_ERROR            = -8
} RTCmixStatus;
    

/*
 [From MMPrint.h]
 
 the different print levels (set by optional param in print_on(lvl)):
 0 -- fatal errors (die() in message.c)
 1 -- print() and printf()'s (in minc/builtin.c)
 2 -- rterrors (rterror() in message.c)
 3 -- warn errors (rtcmix_warn() in message.c)
 4 -- advise notifications (rtcmix_advise() in message.c)
 5 -- all the rest
 6 -- debugging only (rtcmix_debug() in message.c) -- DAS, 12/2013
 
 Brad Garton, 12/2012
*/

/* printing macros */

#define MMP_FATAL			0
#define MMP_PRINTS			1
#define MMP_RTERRORS		2
#define MMP_WARN			3
#define MMP_ADVISE			4
#define MMP_PRINTALL		5
#define MMP_DEBUG			6

#ifdef EMBEDDED
#include "MMPrint.h"
#define RTPrintf(format, ...) set_mm_print_ptr(snprintf(get_mm_print_ptr(), get_mm_print_space(), format, ## __VA_ARGS__)+1)
#define RTFPrintf(FILE, format, ...) set_mm_print_ptr(snprintf(get_mm_print_ptr(), get_mm_print_space(), format, ## __VA_ARGS__)+1)
#define RTPrintfCat(format, ...) set_mm_print_ptr(snprintf(get_mm_print_ptr(), get_mm_print_space(), format, ## __VA_ARGS__))
#define RTFPrintfCat(FILE, format, ...) set_mm_print_ptr(snprintf(get_mm_print_ptr(), get_mm_print_space(), format, ## __VA_ARGS__))
#else
#define RTPrintf(format, ...) printf(format, ## __VA_ARGS__)
#define RTFPrintf(FILE, format, ...) fprintf(FILE, format, ## __VA_ARGS__)
#define RTPrintfCat(format, ...) printf(format, ## __VA_ARGS__)
#define RTFPrintfCat(FILE, format, ...) fprintf(FILE, format, ## __VA_ARGS__)
#endif

#define RTExit(status) throw(status)
    
/* message.cpp */
void rtcmix_debug(const char *inst_name, const char *format, ...);
void rtcmix_advise(const char *inst_name, const char *format, ...);
void rtcmix_warn(const char *inst_name, const char *format, ...);
void rterror(const char *inst_name, const char *format, ...);
void rtcmix_print(const char *format, ...);
/* returns DONT_SCHEDULE if !Option::exitOnError() */
int die(const char *inst_name, const char *format, ...);
RTCmixStatus rtOptionalThrow(RTCmixStatus status);

/* handle the fact that IOS does not allow a call to system() */
#ifndef IOS
#define rt_system(cmd) system(cmd)
#else
#define rt_system(cmd) rtcmix_warn("system", "Not allowed in iOS - not performing command '%s", cmd)
#endif

// pgen function declarations
float *ploc(int tag);
int psize(int tag);
int piloc(int instnum);


int registerFunction(const char *function, const char *dsoName);

void addLegacyfunc(const char *label, double (*func_ptr)(float *, int, double *));

#define STRING_TO_DOUBLE(string) (double)(size_t)(const char *)(string)
#define DOUBLE_TO_STRING(d) (char *)(size_t)(d)
#define STRINGIFY(d) (double)(size_t)(d)

#ifdef __cplusplus
} /* extern "C" */
#endif

#if defined(__cplusplus)
#define UG_INTRO(flabel, func) \
   { \
	addLegacyfunc(flabel, (double (*)(float *, int, double *)) func); \
   }
#else
#define UG_INTRO(flabel, func) \
   { \
      extern double func(); \
      addLegacyfunc(flabel, (double (*)(float *, int, double *)) func); \
   }
#endif	/* __cplusplus */

#endif /* _UGENS_H_ */
