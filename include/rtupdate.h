
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
typedef enum {
  NONE,
  RISE,
  SUSTAIN,
  DECAY
} EnvType;

#ifdef MAIN 
#define GLOBAL
#else
#define  GLOBAL extern
#endif


GLOBAL int curtag;                /* current note tag */
GLOBAL int tags_on;               /* using note tags for rtupdates */
GLOBAL int tag_sem;
GLOBAL int curinst;				  // current instrument tag
GLOBAL int curgen;				  // current gen tag


struct inst_list
{
	int num;
	char* name;
	struct inst_list *next;
}; //inst_list

GLOBAL struct inst_list *ilist;  // list of instrument-name/tag pairs
GLOBAL struct inst_list *genlist; // list of gen-name/tag pairs

#ifdef RTUPDATE

/* contains the values to be updated -- a recirculating array */
GLOBAL float pupdatevals[MAXPUPARR][MAXPUPS];  

// used to store note_pfield_path data
// usage is pfpath[(the tag to operate on)][(the parameter to update)]
// [(the current time-value pair that you're working on)][(0 or 1 to specify
// time(0) or value(1)]
GLOBAL double pfpath[MAXNUMTAGS][MAXNUMPARAMS][MAXPARRAYSIZE][2];

// number of time-value pairs for each tag, paramater, and call to note_pfield
// path
GLOBAL int parray_size[MAXNUMTAGS][MAXNUMPARAMS][MAXNUMCALLS];

// specifies which pgen call to use when interpolating between time-value pairs
// for each tag, parameter, and call to note_pfield_path
GLOBAL int gen_type[MAXNUMTAGS][MAXNUMPARAMS][MAXNUMCALLS];

// used to allow multiple calls to the same instrument tag (works much like 
// the f_goto array in the code for makegen)
GLOBAL int pi_goto[MAXNUMINSTS];

// used to store inst_pfield_path data
GLOBAL double pipath[MAXNUMINSTS][MAXNUMPARAMS][MAXPARRAYSIZE][2];

// number of time-value pairs for each inst, paramater, and call to inst_pfield
// path
GLOBAL int piarray_size[MAXNUMINSTS][MAXNUMPARAMS][MAXNUMCALLS];

// which pgen function to use for above
GLOBAL int igen_type[MAXNUMINSTS][MAXNUMPARAMS][MAXNUMCALLS];

// this information is used to allow multiple calls to note_pfield_path and 
// inst_pfield_path to be cumulative calls and not just have one overwrite the
// other
GLOBAL int numcalls[MAXNUMTAGS][MAXNUMPARAMS];
GLOBAL int cum_parray_size[MAXNUMTAGS][MAXNUMPARAMS];

GLOBAL int numinstcalls[MAXNUMINSTS][MAXNUMPARAMS];
GLOBAL int cum_piarray_size[MAXNUMINSTS][MAXNUMPARAMS];

#endif /* RTUPDATE */

#endif /* _RTUPDATE_H_ */
