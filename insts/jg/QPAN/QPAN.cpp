/* QPAN - 4-channel intensity panner

   The listener is in the center of the coordinate space, at 0,0.
   Left-to-right panning (X) is from -1 to 1; back-to-front panning (Y)
   is from -1 to 1.

      p0 = output start time
      p1 = input start time
      p2 = duration
      p3 = amplitude multiplier
      p4 = X coordinate of virtual source [-1: left <-> 1: right]
      p5 = Y coordinate of virtual source [-1: back <-> 1: front]
      p6 = input channel [optional, default is 0]

   p3 (amplitude), p4 (srcX) and p5 (srcY) can receive dynamic
   updates from a table or real-time control source.

   John Gibson, 11/18/04
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>   // for DBL_MAX
#include <ugens.h>
#include <PField.h>
#include "QPAN.h"
#include <rt.h>
#include <rtdefs.h>

//#define DEBUG

#define PI_OVER_4    (M_PI / 4.0)


QPAN::QPAN()
{
   in = NULL;
   branch = 0;
   src_x = DBL_MAX;
   src_y = DBL_MAX;

   // speakers at 4 corners of a square
   speaker_angles[0] = PI_OVER_4 * 3;     // front left
   speaker_angles[1] = PI_OVER_4;         // front right
   speaker_angles[2] = -PI_OVER_4 * 3;    // back left (NB: unused value)
   speaker_angles[3] = -PI_OVER_4;        // back right
}


QPAN::~QPAN()
{
   delete [] in;
}


int QPAN::usage()
{
   return die("QPAN",
              "Usage: QPAN(start, inskip, dur, amp, srcX, srcY [, inchan]");
}


int QPAN::init(double p[], int n_args)
{
   if (n_args < 6 || n_args > 7)
      return usage();
   float outskip = p[0];
   float inskip = p[1];
   float dur = p[2];
   inchan = n_args > 6 ? (int) p[6] : 0;                 // default is chan 0

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;
   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;

   if (outputChannels() != 4)
      return die("QPAN", "Must have 4 output channels.");

   if (inchan >= inputChannels())
      return die("QPAN", "You asked for channel %d of %d-channel input.",
                                                   inchan, inputChannels());

   return nSamps();
}


const inline double clamp(const double min, const double val, const double max)
{
   if (val < min)
      return min;
   if (val > max)
      return max;
   return val;
}

void QPAN::doupdate()
{
   double p[6];
   update(p, 6, kAmp | kSrcX | kSrcY);

   amp = p[3];

   const double x = p[4];
   const double y = p[5];

   if (x != src_x || y != src_y) {
      src_x = clamp(-1.0, x, 1.0);
      src_y = clamp(-1.0, y, 1.0);

      const double src_x_angle = atan2(1.0, src_x);
      const double src_y_angle = atan2(src_y, 1.0);

      const double left = cos(speaker_angles[0] - src_x_angle);
      const double right = cos(src_x_angle - speaker_angles[1]);

      const double front = cos(speaker_angles[1] - src_y_angle);
      const double back = cos(src_y_angle - speaker_angles[3]);

      gains[0] = left * front;
      gains[1] = right * front;
      gains[2] = left * back;
      gains[3] = right * back;

#ifdef DEBUG
      rtcmix_advise("QPAN",
             "gains:  l=%.3f, r=%.3f, f=%.3f, b=%.3f [%.3f, %.3f, %.3f, %.3f]",
             left, right, front, back,
             gains[0], gains[1], gains[2], gains[3]);
#endif
   }
}


int QPAN::configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


int QPAN::run()
{
   float out[outputChannels()];
   const int inchans = inputChannels();
   const int outchans = outputChannels();
   const int samps = framesToRun() * inchans;

   rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inchans) {
      if (--branch <= 0) {
         doupdate();
         branch = getSkip();
      }
      float insig = in[i + inchan] * amp;

      for (int j = 0; j < outchans; j++)
         out[j] = insig * gains[j];

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


Instrument *makeQPAN()
{
   QPAN *inst = new QPAN();
   inst->set_bus_config("QPAN");

   return inst;
}


void rtprofile()
{
   RT_INTRO("QPAN", makeQPAN);
}

