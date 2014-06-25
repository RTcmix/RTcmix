#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <float.h>   // for DBL_MAX
#include <ugens.h>
#include "speakers.h"

#define MIN_DISTANCE_FACTOR 0.25    // distance of 0.25 quadruples gain

#define TWO_PI    (M_PI * 2.0)

static int _num_speakers = 0;
static Speaker **_speakers;


Speaker::Speaker(const int channel, const double angle, const double distance)
   : _channel(channel), _angle(angle), _prevAngle(0.0), _nextAngle(0.0),
     _distance(distance), _gain(0.0)
{
}

Speaker::~Speaker()
{
}


// ------------------------------------------------------- NPAN_get_speakers ---
// Fill the given array of speakers, and pass back the number of speakers.
// Return 0 if okay, -1 if user has not specified speaker locations in a call
// to NPANspeakers.  Caller is responsible for deleting all of the array
// elements and the array itself.

int
NPAN_get_speakers(int *numspeakers, Speaker *speakers[], double *min_distance)
{
   if (_num_speakers == 0)
      return -1;

   double min = DBL_MAX;

   // Each client inst needs private copies of the speaker objects.
   for (int i = 0; i < _num_speakers; i++) {
      const double distance = _speakers[i]->distance();
      if (distance < min)
         min = distance; 
      speakers[i] = new Speaker(_speakers[i]->channel(), _speakers[i]->angle(),
                                distance);
      speakers[i]->setPrevAngle(_speakers[i]->prevAngle());
      speakers[i]->setNextAngle(_speakers[i]->nextAngle());
   }

   *numspeakers = _num_speakers;
   *min_distance = min * MIN_DISTANCE_FACTOR;

   return 0;
}


// ------------------------------------------------------------------- usage ---
static double
usage()
{
   return die("NPANspeakers", "Usage:\n"
      "   NPANspeakers(\"polar\", "
          "spk1_angle, spk1_dist ... spkN_angle, spkN_dist)\n"
      "   (where 0 is directly in front of listener)\n"
      "or\n"
      "   NPANspeakers(\"xy\", "
          "spk1_x, spk1_y ... spkN_x, spkN_y)");
}


extern "C" {

// ---------------------------------------------------------- compare_floats ---
// Comparison function for qsort call below.
static int compare_speaker_angles(const void *a, const void *b)
{
   const Speaker *spk1 = (Speaker *) *((int *) a);
   const Speaker *spk2 = (Speaker *) *((int *) b);

//printf("compare: spk1: %f spk2: %f\n", spk1->angle(), spk2->angle());

   if (spk1->angle() > spk2->angle())
      return 1;
   else if (spk1->angle() < spk2->angle())
      return -1;
   return 0;
}


// ---------------------------------------------------------------- speakers ---
// Script function to set up NPAN params.
//
//    p0 = mode: "polar" or "xy" ("cartesian")
//
// (Actually, any string not beginning with "pol" specifies cartesian mode.)
//
// Our polar coords differs from the normal arrangement, with 0 to the right.
// Instead, 0 is in front of the listener, where 90 degrees normally is.

double
NPAN_set_speakers(float p[], int nargs, double pp[])
{
   if (nargs < 5 || !(nargs % 2))
      return usage();

   if (_num_speakers > 0) {
      for (int i = 0; i < _num_speakers; i++) // delete ones from previous call
         delete _speakers[i];
      delete [] _speakers;
   }

   _num_speakers = (nargs - 1) / 2;
   if (_num_speakers < 2)
      return die("NPANspeakers", "Must have at least 2 speakers.");
   if (_num_speakers > MAX_SPEAKERS)
      return die("NPANspeakers", "Can't have more than %d speakers.",
                                                            MAX_SPEAKERS);

   _speakers = new Speaker * [_num_speakers];

   char *modestr = DOUBLE_TO_STRING(pp[0]);
   if (strncmp(modestr, "pol", 3) == 0) {          // polar coordinates
      // Convert from user coordinates by adding 90 degrees, then convert
      // to radians.  Finally, normalize radians to [-PI, PI].
      int j = 1;
      for (int i = 0; i < _num_speakers; i++, j += 2) {
         const double degrees = pp[j] + 90.0;      // user to internal degrees
         double angle = M_PI * 2 * (degrees / 360.0);    // to radians
         angle = atan2(sin(angle), cos(angle));    // normalize
         const double distance = pp[j + 1];
         _speakers[i] = new Speaker(i, angle, distance);
      }
   }
   else {                                          // cartesian coordinates
      int j = 1;
      for (int i = 0; i < _num_speakers; i++, j += 2) {
         const double x = pp[j];
         const double y = pp[j + 1];
         const double angle = atan2(y, x);
         const double distance = sqrt((x * x) + (y * y));
         _speakers[i] = new Speaker(i, angle, distance);
      }
   }

   // sort speaker array by ascending angle
   qsort(_speakers, _num_speakers, sizeof(Speaker *), compare_speaker_angles);

   // cache angle of adjacent speaker
   for (int i = 0; i < _num_speakers; i++) {
      if (i == 0)
         _speakers[i]->setPrevAngle(_speakers[_num_speakers - 1]->angle()
                                                                  - TWO_PI);
      else
         _speakers[i]->setPrevAngle(_speakers[i - 1]->angle());
      if (i == _num_speakers - 1)
         _speakers[i]->setNextAngle(_speakers[0]->angle() + TWO_PI);
      else
         _speakers[i]->setNextAngle(_speakers[i + 1]->angle());
   }

   // check for two speakers having same angle
   for (int i = 0; i < _num_speakers; i++) {
      if (_speakers[i]->nextAngle() == _speakers[i]->angle())
         return die("NPANspeakers", "Can't put two speakers in same location.");
   }

   return 0.0;
}


// ----------------------------------------------------------------- profile ---
int
profile()
{
   UG_INTRO("NPANspeakers", NPAN_set_speakers);
   return 0;
}

} // extern "C"

