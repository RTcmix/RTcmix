/*
 *	ugens.h: define unit generator structure
 */
#ifndef _UGENS_H_ 
#define _UGENS_H_ 1

#define	UG_NSIZ	7	/*  Max len of UG name	*/
#define NAMESIZE 128    /* Max size of file name */
#define	UG_NULL	(struct ug_item *)0

#define FLOAT (sizeof(float))   /* nbytes in floating point word*/
#define INT   (sizeof(int))   /* nbytes in integer word */
#define SHORT (sizeof(short))
#define LONG  (sizeof(long))
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

extern	ug_t	*ug_list;
#ifndef PFIELD_CLASS
int addfunc(struct ug_item *);
#endif
void merror(char *);

#ifdef PFIELD_CLASS
#include <ug_intro.h>
#else /* !PFIELD_CLASS */
void ug_intro(void);
#ifdef __cplusplus
#define	UG_INTRO(flabel,func)	\
	{ \
		static ug_t this_ug = { UG_NULL, (double (*)()) func, flabel }; \
		if (addfunc(&this_ug) == -1) merror(flabel);	}
#else
#define	UG_INTRO(flabel,func)	\
	{ extern double func();	\
		static ug_t this_ug = { UG_NULL, func, flabel }; \
		if (addfunc(&this_ug) == -1) merror(flabel);	}
#endif
#endif /* !PFIELD_CLASS */


#ifndef PI
#define      PI     3.141592654
#endif
#ifndef PI2
#define      PI2    6.2831853
#endif

extern float SR;

/* times per second to update control-rate variables;
   defined in sys/minc_functions.c
*/
extern int resetval;

 
/*  structure to pass to gen routines */
struct gen {
           int size;           /* size of array to load up */
           int nargs;          /* number of arguments passed in p array */
           float *pvals;       /* address of array of p values */
           float *array;       /* address of array to be loaded up */
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
float *resample_gen(float [], int, int, InterpolationType);

typedef enum {
   ADD_GENS,
   MULT_GENS
} GenModType;
int combine_gens(int, int, int, int, GenModType, char *);
int install_gen(int, int, float *);
double makegen(float [], int, double []);
float *floc(int);
int fsize(int);

extern int (*getsample)();
#define GETSAMPLE (*getsample)

float allpass(float, float *);
float allpole(float, int*, int, float*, float*);
double ampdb(float);
double dbamp(float);
float *ballpole(float*, long*, long, float*, float*, float*, long);
float *bbuzz(float, float, float, float*, float*, float*, long);
float boost(float);
int boscili(float, float, float*, int, float*, float*, int);
float bpluck(float, float*);
float breson(float*, float*, float*, int);
void sbrrand(unsigned int);
void brrand(float, float*, int);
float buzz(float, float, float, float*, float*);
float comb(float, float*);
void combset(float, float,int, float*);
float cpsoct(float);
float cpspch(float);
float delget(float*, float, int*);
void delput(float, float*, int*);
void delset(float*, int*, float);
float dliget(float*, float, int*);
float evp(long, float*, float*, float*);
void evset(float, float, float, int, float*);
float hcomb(float,float,float*);
void hplset(float, float, float, float, float, float, int, float*);
float hpluck(float, float*);
float midipch(float);
float octcps(float);
float octpch(float);
float oscil(float, float, float*, int, float*);
float oscili(float, float, float*, int, float*);
float osciln(float, float, float*, int, float*);
float oscilni(float, float, float*, int, float*);
float pchcps(float);
float pchmidi(unsigned char);
float pchoct(float);
float pluck(float, float*);
void pluckset(float, float, float, float, float*, float);
float randf(float*, float);
float crandom(float);
float reson(float, float*);
void rsnset(float, float, float, float, float*);
void rszset(float, float, float, float*);
float resonz(float, float*);
void bresonz(float*, float*, float*, int);
float reverb(float, float*);
void setline(float [], short, int, float []);
void srrand(unsigned int);
void rvbset(float, int, float*);
float table(long, float*, float*);
float tablei(long, float*, float*);
void tableset(float, int, float*);
float wshape(float, float*, int);
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
int _readit(int fno);
int _writeit(int fno);
void _backup(int fno);
void _forward(int fno);
void closesf(void);
void closesf_noexit(void);

/* minout.c */
int inrepos(int samps, int fno);
int outrepos(int samps, int fno);

/* fnscl.c */
void fnscl(struct gen *gen);

/* message.c */
void advise(const char *inst_name, const char *format, ...);
void warn(const char *inst_name, const char *format, ...);
void rterror(const char *inst_name, const char *format, ...);
void die(const char *inst_name, const char *format, ...);

// pgen function declarations
float *ploc(int tag);
int psize(int tag);
int piloc(int instnum);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* _UGENS_H_ */
