/* Base class for envelope follower instruments
   John Gibson <johgibso at indiana dot edu>, 1/5/03.
*/

//#define DEBUG
#include "FOLLOWER_BASE.h"


/* --------------------------------------------------------- FOLLOWER_BASE -- */
FOLLOWER_BASE :: FOLLOWER_BASE()
{
   branch = 0;
   in = NULL;
   amp_table = NULL;
   smoother = NULL;
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
   int   window_len;
   float *function, outskip, inskip, smoothness;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   caramp = amp = p[3];

   /* apply conversion to normal range [-1, 1] for modulator input now */
   modamp = p[4] / 32768.0;

   window_len = (int) p[5];
   if (window_len < 1)
      return die(instname(), "Window length must be at least 1 sample.");

   smoothness = p[6];
   if (smoothness < 0.0 || smoothness > 1.0)
      return die(instname(), "Smoothness must be between 0 and 1.");

   /* Filter pole coefficient is non-linear -- very sensitive near 1, and
      not very sensitive below .5, so we use a log to reduce this nonlinearity
      for the user.  Constrain to range [1, 100], then take log10.
   */
   smoothness = (float) log10((double) ((smoothness * 99) + 1)) * 0.5f;
   if (smoothness > 0.9999)   /* 1.0 results in all zeros for power signal */
      smoothness = 0.9999;
   DPRINT1("smoothness: %f\n", smoothness);

   if (pre_init(p, n_args) != 0)
      return DONT_SCHEDULE;

   rtsetoutput(outskip, dur, this);
   if (outputChannels() > 2)
      return die(instname(), "Output must be either mono or stereo.");

   if (rtsetinput(inskip, this) != 0)
      return DONT_SCHEDULE;

   if (inputChannels() != 2)
      return die(instname(),
      "Must use 2 input channels: 'left' for carrier; 'right' for modulator.");

   function = floc(1);
   if (function) {
      int len = fsize(1);
      amp_table = new TableL(dur, function, len);
   }
   else
      advise(instname(), "Setting flat amplitude curve, since no table 1.");

   gauge = new RMS();
   gauge->setWindowSize(window_len);

   if (smoothness > 0.0) {
      smoother = new OnePole();
      smoother->setPole(smoothness);
   }

   skip = (int) (SR / (float) resetval);

   if (post_init(p, n_args) != 0)
      return DONT_SCHEDULE;

   return nSamps();
}


/* ------------------------------------------------------------------- run -- */
int FOLLOWER_BASE :: run()
{
   float out[2];

   if (in == NULL)            /* first time, so allocate it */
      in = new float [RTBUFSAMPS * inputChannels()];

   const int insamps = framesToRun() * inputChannels();
   rtgetin(in, this, insamps);

   for (int i = 0; i < insamps; i += inputChannels()) {
      if (--branch < 0) {
         if (amp_table)
            caramp = amp_table->tick(currentFrame(), amp);
         update_params();
         branch = skip;
      }
      float carsig = in[i] * caramp;
      float modsig = in[i + 1] * modamp;
      float power = gauge->tick(modsig);
      if (smoother)
         power = smoother->tick(power);
      float outsig = process_sample(carsig, power);
      if (outputChannels() == 2) {
         out[0] = outsig * pctleft;
         out[1] = outsig * (1.0 - pctleft);
      }

      rtaddout(out);
      increment();
   }

   return framesToRun();
}


