/* Base class for envelope follower instruments
   John Gibson <johgibso at indiana dot edu>, 1/5/03.  Rev for v4, 7/12/04
*/

//#define DEBUG
#include "FOLLOWER_BASE.h"
#include <float.h>


/* --------------------------------------------------------- FOLLOWER_BASE -- */
FOLLOWER_BASE :: FOLLOWER_BASE()
   : branch(0), in(NULL), rawmodamp(-FLT_MAX), amp_table(NULL)
{
}


/* -------------------------------------------------------- ~FOLLOWER_BASE -- */
FOLLOWER_BASE :: ~FOLLOWER_BASE()
{
   delete [] in;
   delete amp_table;
   delete gauge;
   delete smoother;
}


/* ------------------------------------------------------------------ init -- */
int FOLLOWER_BASE :: init(double p[], int n_args)
{
   nargs = n_args;
   float outskip = p[0];
   float inskip = p[1];
   dur = p[2];

   int window_len = (int) p[5];
   if (window_len < 1)
      return die(instname(), "Window length must be at least 1 sample.");

   smoothness = p[6];
   if (smoothness < 0.0 || smoothness > 1.0)
      return die(instname(), "Smoothness must be between 0 and 1.");
   smoothness = -1.0;   // force first update

   if (pre_init(p, n_args) != 0)
      return DONT_SCHEDULE;

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;
   if (outputChannels() > 2)
      return die(instname(), "Output must be either mono or stereo.");

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;

   if (inputChannels() != 2)
      return die(instname(),
      "Must use 2 input channels: 'left' for carrier; 'right' for modulator.");

   double *function = floc(1);
   if (function) {
      int len = fsize(1);
      amp_table = new TableL(SR, dur, function, len);
   }

   gauge = new RMS(SR);
   gauge->setWindowSize(window_len);

   smoother = new JGOnePole(SR);

   if (post_init(p, n_args) != 0)
      return DONT_SCHEDULE;

   return nSamps();
}


/* ------------------------------------------------------------- configure -- */
int FOLLOWER_BASE :: configure()
{
   in = new float [RTBUFSAMPS * inputChannels()];
   return in ? 0 : -1;
}


/* ------------------------------------------------------- update_smoother -- */
inline void FOLLOWER_BASE :: update_smoother(double p[])
{
   if (p[6] != smoothness) {
      smoothness = p[6];
      if (smoothness < 0.0)
         smoothness = 0.0;
      else if (smoothness > 1.0)
         smoothness = 1.0;

      // Filter pole coefficient is non-linear -- very sensitive near 1,
      // and not very sensitive below .5, so we use a log to reduce this
      // nonlinearity for the user.  Constrain to range [1, 100], then
      // take log10.

      float smooth = (float) log10((double) ((smoothness * 99) + 1)) * 0.5f;
      if (smooth > 0.9999)    // 1.0 results in all zeros for power signal
         smooth = 0.9999;

      smoother->setPole(smooth);
   }
}


/* ------------------------------------------------------------------- run -- */
int FOLLOWER_BASE :: run()
{
   const int samps = framesToRun() * inputChannels();
   rtgetin(in, this, samps);

   for (int i = 0; i < samps; i += inputChannels()) {
      if (--branch <= 0) {
         double p[nargs];
         update(p, nargs);
         caramp = p[3];
         if (amp_table)
            caramp *= amp_table->tick(currentFrame(), 1.0);
         if (p[4] != rawmodamp) {
            rawmodamp = p[4];
            // apply conversion to normal range [-1, 1]
            modamp = rawmodamp * (1.0 / 32768.0);
         }
         update_smoother(p);
         update_params(p);
         branch = getSkip();
      }
      float carsig = in[i] * caramp;
      float modsig = in[i + 1] * modamp;
      float power = gauge->tick(modsig);
      power = smoother->tick(power);
      float outsig = process_sample(carsig, power);

      float out[2];
      if (outputChannels() == 2) {
         out[0] = outsig * pctleft;
         out[1] = outsig * (1.0 - pctleft);
      }
		else
			out[0] = outsig;

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


