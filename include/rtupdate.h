
#ifndef _RTUPDATE_H_ 
#define _RTUPDATE_H_ 1

#define MAXPUPARR 100
#define MAXPUPS 20
#define NOPUPDATE 78787878 // hopefully never a real p-value!

#define MAXNUMTAGS 100
#define MAXNUMPARAMS 100
#define MAXPARRAYSIZE 100

#define MAXNUMINSTS 100

#define MAXNUMCALLS 10

/* For rtperf */
/* DJT:  modified to work with JG's objlib.h */
#ifndef USE_ADSR
typedef enum {
  NONE,
  RISE,
  SUSTAIN,
  DECAY
} EnvType;
#endif

#ifdef MAIN 
#define GLOBAL
#else
#define  GLOBAL extern
#endif
GLOBAL int curtag;                /* current note tag */
GLOBAL int tags_on;               /* using note tags for rtupdates */
GLOBAL int tag_sem;
GLOBAL int curinst;

struct inst_list
{
	int num;
	char* name;
	struct inst_list *next;
}; //inst_list

GLOBAL struct inst_list *ilist;
/* contains the values to be updated -- a recirculating array */

GLOBAL float pupdatevals[MAXPUPARR][MAXPUPS];

GLOBAL int parray_size[MAXNUMTAGS][MAXNUMPARAMS][MAXNUMCALLS];
GLOBAL int gen_type[MAXNUMTAGS][MAXNUMPARAMS][MAXNUMCALLS];
GLOBAL double pfpath[MAXNUMTAGS][MAXNUMPARAMS][MAXPARRAYSIZE][2];

GLOBAL int pi_goto[MAXNUMINSTS];
GLOBAL int piarray_size[MAXNUMINSTS][MAXNUMPARAMS][MAXNUMCALLS];
GLOBAL int igen_type[MAXNUMINSTS][MAXNUMPARAMS][MAXNUMCALLS];
GLOBAL double pipath[MAXNUMINSTS][MAXNUMPARAMS][MAXPARRAYSIZE][2];

GLOBAL int numcalls[MAXNUMTAGS][MAXNUMPARAMS];
GLOBAL int cum_parray_size[MAXNUMTAGS][MAXNUMPARAMS];

GLOBAL int numinstcalls[MAXNUMINSTS][MAXNUMPARAMS];
GLOBAL int cum_piarray_size[MAXNUMINSTS][MAXNUMPARAMS];
#endif
