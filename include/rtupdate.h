
#ifndef _RTUPDATE_H_ 
#define _RTUPDATE_H_ 1

#define MAXPUPARR 100
#define MAXPUPS 20
#define NOPUPDATE 78787878 // hopefully never a real p-value!

#define MAXNUMTAGS 1000
#define MAXNUMPARAMS 100
#define MAXPARRAYSIZE 100

#define MAXNUMINSTS 100
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

GLOBAL int parray_size[MAXNUMTAGS][MAXNUMPARAMS];
GLOBAL int gen_type[MAXNUMTAGS][MAXNUMPARAMS];
GLOBAL double pfpath[MAXNUMTAGS][MAXNUMPARAMS][MAXPARRAYSIZE][2];

GLOBAL int pi_goto[MAXNUMINSTS];
GLOBAL int piarray_size[MAXNUMINSTS][MAXNUMPARAMS];
GLOBAL int igen_type[MAXNUMINSTS][MAXNUMPARAMS];
GLOBAL double pipath[MAXNUMINSTS][MAXNUMPARAMS][MAXPARRAYSIZE][2];

#endif
