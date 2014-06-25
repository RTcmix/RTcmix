/* NPAN - multichannel pair-wise intensity panner

   First call NPANspeakers to configure the number of speakers and
   their locations.

      NPANspeakers(mode, speak1_a, speak1_b ... speakN_a, speakN_b)

   where mode is "polar" or "xy" (or "cartesian"), followed by two or more
   pairs of arguments defining speaker placement relative to the listener.

   The listener is in the center of the coordinate space, at 0,0.  Angles
   are measured in degrees, with 0 degrees directly in front of the listener,
   90 degrees to their left and -90 degrees to their right.

   The order of speakers in the argument list is significant: the order
   corresponds to the order of output channels.  So if you want the front
   left speaker in the first output channel, list it first.

   If mode is polar,
      speakN_a is the angle of the speaker relative to the listener
      speakN_b is the distance of the speaker from the listener

   If mode is cartesian,
      speakN_a is the X coordinate of the speaker
      speakN_b is the Y coordinate of the speaker


   Then call NPAN itself to do the actual panning.

      p0 = output start time
      p1 = input start time
      p2 = duration
      p3 = global amplitude multiplier
      p4 = mode: "polar" or "xy" (or "cartesian")

   p5 and p6 give the position of the virtual sound source, relative
   to the listener, using the coordinate space described above.
   The interpretation of p5 and p6 depends on mode (p4).

   If mode is polar,
      p5 = angle (in degrees), relative to listener
      p6 = distance from listener

   If mode is cartesian,
      p5 = X coordinate
      p6 = Y coordinate

      p7 = input channel [optional, default is 0]

   p3 (amplitude), p5 (X | angle) and p6 (Y | radius) can receive dynamic
   updates from a table or real-time control source.

   This is pair-wise panning, so no more than two adjacent speakers have
   signal at any time.  It is not possible to place a virtual source between
   two speakers that are not adjacent.

   Distance from the listener affects gain (the closer, the higher the gain).
   Very short distances can cause clipping.  If in doubt, use the polar mode,
   set both speaker distances and virtual source distance to 1.

   ----

   Based on the description in F. R. Moore, "Elements of Computer Music,"
   pp. 353-9.

   John Gibson, 11/13/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>   // for DBL_MAX
#include <ugens.h>
#include <Instrument.h>
#include <PField.h>
#include "NPAN.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG

#define TWO_PI       (M_PI * 2.0)
#define PI_OVER_2    (M_PI / 2.0)


NPAN::NPAN() : Instrument()
{
   in = NULL;
   num_speakers = 0;
   prev_angle = -DBL_MAX;
   src_x = DBL_MAX;
   src_y = DBL_MAX;
   branch = 0;
}


NPAN::~NPAN()
{
   delete [] in;
   for (int i = 0; i < num_speakers; i++)
      delete speakers[i];
}


int NPAN::usage()
{
   return die("NPAN",
              "Usage: NPAN(start, inskip, dur, amp, \"polar\" | \"xy\", "
              "srcAngle | srcX, srcDistance | srcY [, inchan]");
}


int NPAN::getmode()
{
   const PField &field = getPField(4);
   const char *str = field.stringValue(0);
   if (str == NULL)
      return -1;
   if (strncmp(str, "pol", 3) == 0)
      return PolarMode;
   return CartesianMode;
}


void NPAN::dumpspeakers()
{
   rtcmix_advise("NPAN", "Speakers (angles are internal degrees) -------------------");
   for (int i = 0; i < num_speakers; i++) {
      rtcmix_advise("NPAN", "chan %d:\tangle=%g, nextAngle=%g distance=%g",
         speakers[i]->channel(),
         speakers[i]->angle(),
         speakers[i]->nextAngle(),
         speakers[i]->distance());
   }
}


int NPAN::init(double p[], int n_args)
{
   if (n_args < 7)
      return usage();
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   mode = getmode();
   if (mode == -1)
      return usage();
   inchan = n_args > 7 ? (int) p[7] : 0;                 // default is chan 0

   if (NPAN_get_speakers(&num_speakers, speakers, &min_distance) == -1)
      return die("NPAN",
                 "Call NPANspeakers before NPAN to set up speaker locations.");
#ifdef DEBUG
   dumpspeakers();
#endif

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;
   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;

   if (outputChannels() != num_speakers)
      return die("NPAN", "Must have one output channel for each speaker.");

   if (inchan >= inputChannels())
      return die("NPAN", "You asked for channel %d of a %d-channel file.",
                                                   inchan, inputChannels());

   skip = (int) (SR / (float) resetval);

   return nSamps();
}


void NPAN::setgains()
{
#ifdef DEBUG
   rtcmix_advise("NPAN", "----------------------------------------------------------");
   rtcmix_advise("NPAN", "src: x=%f, y=%f, angle=%g, dist=%g",
      src_x, src_y, src_angle / TWO_PI * 360.0, src_distance);
#endif
   const double pi_over_2 = PI_OVER_2;

   // Minimum distance from listener to source; don't get closer than this.
   if (src_distance < min_distance)
      src_distance = min_distance;

   // Speakers are guaranteed to be in ascending angle order, from -PI to PI.
   for (int i = 0; i < num_speakers; i++) {
      const double spk_angle = speakers[i]->angle();
      const double spk_prev_angle = speakers[i]->prevAngle();
      const double spk_next_angle = speakers[i]->nextAngle();

      // Handle angle wraparound for first and last speakers
      double source_angle = src_angle;
      if (i == 0 && src_angle > 0.0)
         source_angle -= TWO_PI;
      else if (i == num_speakers - 1 && src_angle < 0.0)
         source_angle += TWO_PI;

      if (source_angle > spk_prev_angle && source_angle < spk_next_angle) {
         // then this speaker gets some signal

         // Scale difference between src angle and speaker angle so that
         // max range is [0, 1].
         double scale;
         if (source_angle < spk_angle)
            scale = (spk_angle - spk_prev_angle) / pi_over_2;
         else
            scale = (spk_next_angle - spk_angle) / pi_over_2;
         const double diff = fabs(source_angle - spk_angle) / scale;

         // Gain is combination of src angle and distance, rel. to speaker.
         const double distfactor = speakers[i]->distance() / src_distance;
         speakers[i]->setGain(cos(diff) * distfactor);
      }
      else
         speakers[i]->setGain(0.0);

#ifdef DEBUG
      rtcmix_advise("NPAN", "speaker[%d]: chan=%d, angle=%g, dist=%g, gain=%.12f",
             i, speakers[i]->channel(), speakers[i]->angleDegrees(),
             speakers[i]->distance(), speakers[i]->gain());
#endif
   }
}


void NPAN::doupdate()
{
   double p[7];
   update(p, 7);

   amp = p[3];

   if (mode == PolarMode) {
      double angle = p[5];
      const double dist = p[6];
      if (angle != prev_angle || dist != src_distance) {
         prev_angle = angle;
         angle += 90.0;                               // user -> internal
         angle *= TWO_PI / 360.0;                     // degrees -> radians
         src_angle = atan2(sin(angle), cos(angle));   // normalize to [-PI, PI]
         src_distance = dist;
         src_x = cos(src_angle) * dist;
         src_y = sin(src_angle) * dist;
         setgains();
      }
   }
   else {
      const double x = p[5];
      const double y = p[6];
      if (x != src_x || y != src_y) {
         src_x = x;
         src_y = y;
         src_angle = atan2(src_y, src_x);
         src_distance = sqrt((src_x * src_x) + (src_y * src_y));
         setgains();
      }
   }
}


int NPAN::configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int NPAN::run()
{
   const int outchans = outputChannels();
   float out[outchans];
   const int samps = framesToRun() * inputChannels();

   rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         doupdate();
         branch = skip;
      }
      float insig = in[i + inchan] * amp;

      for (int j = 0; j < outchans; j++)
         out[speakers[j]->channel()] = insig * speakers[j]->gain();

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeNPAN()
{
   NPAN *inst;

   inst = new NPAN();
   inst->set_bus_config("NPAN");

   return inst;
}


void rtprofile()
{
   RT_INTRO("NPAN", makeNPAN);
}

