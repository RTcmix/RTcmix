#ifndef _RTUPDATE_H_ 
#define _RTUPDATE_H_ 1

#define MAXPUPARR 100
#define MAXPUPS 20
#define NOPUPDATE 78787878 // hopefully never a real p-value!

/* For rtperf */
/* DJT:  modified to work with JG's objlib.h */
typedef enum {
  NONE,
  RISE,
  SUSTAIN,
  DECAY
} EnvType;

#ifndef GLOBAL
#define GLOBAL extern
#endif
GLOBAL int curtag;                /* current note tag */
GLOBAL int tags_on;               /* using note tags for rtupdates */
GLOBAL int tag_sem;

/* contains the values to be updated -- a recirculating array */
GLOBAL float pupdatevals[MAXPUPARR][MAXPUPS];

#endif
