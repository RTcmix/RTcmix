/*
 *	ugens.h: define unit generator structure
 */
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


extern	ug_t	*ug_list;
extern	addfunc(struct ug_item *);

#define	UG_INTRO(flabel,func)	\
	{ extern double func();	\
		static ug_t this_ug = { UG_NULL, func, flabel }; \
		if (addfunc(&this_ug) == -1) merror(flabel);	}


#ifndef PI
#define      PI     3.141592654
#endif
#ifndef PI2
#define      PI2    6.2831853
#endif

extern       float  SR;

#ifndef MAIN
extern int aargc;
extern char *aargv[];          /* to pass commandline args to subroutines */
#endif

 
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
float *floc(int);
int fsize(int);

extern int (*getsample)();
#define GETSAMPLE (*getsample)


float allpass(float, float *);
float allpole(float, int*, int, float*, float*);
double ampdb(float);
float *ballpole(float*, long*, long, float*, float*, float*, long);
float *bbuzz(float, float, float, float*, float*, float*, long);
float boost(float);
int boscili(float, float, float*, int, float*, float*, int);
float bpluck(float, float*);
float breson(float*, float*, float*, int);
void sbrrand(unsigned);
int brrand(float, float*, int);
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
float bresonz(float*, float*, float*, int);
float reverb(float, float*);
void srrand(unsigned);
void rvbset(float, int, float*);
float table(long, float*, float*);
float tablei(long, float*, float*);
void tableset(float, int, float*);
float wshape(float, float*, int);
/* int rtgetin(float *, int, int);  Perhaps not w/o full-duplex */
float rrand(void);

