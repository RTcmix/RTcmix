#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include "timeset.h"

static int _ntimes = 0;
static float _timepts[TIME_ARRAY_SIZE];
static float _xvals[TIME_ARRAY_SIZE], _yvals[TIME_ARRAY_SIZE];


/* ---------------------------------------------------------- get_timeset --- */
int
get_timeset(float timepts[], float xvals[], float yvals[])
{
   int n;

   if (_ntimes < 2)      /* must have at least two timeset calls */
      return 0;

   for (n = 0; n < _ntimes; n++) {
      timepts[n] = _timepts[n];
      xvals[n] = _xvals[n];
      yvals[n] = _yvals[n];
   }
   _ntimes = 0;          /* zero for next set of timeset calls */

   return n;
}


/* -------------------------------------------------------------- timeset --- */
double
timeset(float p[], int n_args)
{
   if (_ntimes < TIME_ARRAY_SIZE) {
      _timepts[_ntimes] = p[0];
      _xvals[_ntimes] = p[1];
      _yvals[_ntimes] = p[2];
      _ntimes++;
   }
   else
      rtcmix_warn("MROOM", "Can only have %d timeset calls for each MROOM.",
                                                            TIME_ARRAY_SIZE);

   return 0.0;
}

#ifndef MAXMSP
/* -------------------------------------------------------------- profile --- */
int
profile()
{
   UG_INTRO("timeset", timeset);
   return 0;
}
#endif
