/* COMPLIMIT:

      John Gibson (jgg9c@virginia.edu), 3/25/00
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>
#include "COMPLIMIT.h"
#include <rt.h>
#include <rtdefs.h>
#include <assert.h>

//#define DEBUG

extern "C" {
   #include <ugens.h>
   extern int resetval;
}


COMPLIMIT::COMPLIMIT() : Instrument()
{
   in = new float[MAXBUF];

   rms_gauge = NULL;
   env_stage = ENV_INACTIVE;
   env_count = 0;
   peak = 0.0;
   branch = 0;
}


COMPLIMIT::~COMPLIMIT()
{
   delete [] in;
   delete rms_gauge;
}


int COMPLIMIT::init(float p[], short n_args)
{
   int   detector_int;
   float outskip, inskip, dur, atk_time, rel_time;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   ingain = p[3];
   outgain = p[4];
   atk_time = p[5];
   rel_time = p[6];
   threshold = p[7];
   ratio = p[8];
   window_len = (int) p[9];
   detector_int = (int) p[10];
   bypass = (int) p[11];
   inchan = (n_args > 12) ? (int) p[12] : 0;
   pctleft = (n_args > 13) ? p[13] : 0.5;

   rtsetinput(inskip, this);
   nsamps = rtsetoutput(outskip, dur, this);

   if (inchan >= inputchans) {
      fprintf(stderr, "You asked for channel %d of a %d-channel file.\n",
                      inchan, inputchans);
      exit(1);
   }
   if (NCHANS > 2) {
      fprintf(stderr, "Can't use more than 2 output channels.\n");
      exit(1);
   }

   if (atk_time < 0.0 || rel_time < 0.0) {
      fprintf(stderr, "Invalid envelope times.\n");
      exit(1);
   }
   atk_samps = (int) (atk_time * SR);
   rel_samps = (int) (rel_time * SR);
   atk_increment = 1.0 / (float) atk_samps;
   rel_increment = 1.0 / (float) rel_samps;

#ifdef DEBUG
   printf("atk_samps=%d, rel_samps=%d; atk_increment=%g, rel_increment=%g\n",
                         atk_samps, rel_samps, atk_increment, rel_increment);
#endif

#ifdef NOTYET
   if (threshold < 0.0 || threshold > 96.0) {     // FIXME
      fprintf(stderr, "Invalid threshold.\n");
      exit(1);
   }
   threshold = ampdb(threshold);
#endif

   if (ratio < 1.0) {
      fprintf(stderr, "Invalid ratio.\n");
      exit(1);
   }

// FIXME: defaults, etc.
// should call this "response time" or "lookahead time", and quantize it
// internally to fit bufsize
   if (window_len == 0)
      window_len = 128;
   if (window_len > RTBUFSAMPS) {
      fprintf(stderr, "Window length must be less than %d samples (the output "
                      "buffer size). Correcting...\n", RTBUFSAMPS);
      window_len = RTBUFSAMPS;
   }
// FIXME: adjust instead of bailing
   if (RTBUFSAMPS % window_len) {
      fprintf(stderr, "Window length must be a power of two.\n");
      exit(1);
   }

   if (detector_int < 0 || detector_int > 2) {
      fprintf(stderr, "Invalid detector type.\n");
      exit(1);
   }
   detector_type = (DetectType) detector_int;

   if (detector_type == RMS_DETECTOR) {
      rms_gauge = new RMS();
      rms_gauge->setWindowSize(window_len);
   }

   amptable = floc(1);
   if (amptable) {
      int amplen = fsize(1);
      tableset(dur, amplen, amptabs);
   }
   else
      printf("Setting phrase curve to all 1's\n");

   skip = (int) (SR / (float) resetval);

   oamp = 1.0;                  /* in case amptable == NULL */

   return nsamps;
}


inline float COMPLIMIT::get_peak(int offset)
{
   int   endsamp;
   float pk = 0.0;

   endsamp = offset + (window_len * inputchans);

   if (detector_type == RMS_DETECTOR) {
      for (int i = offset; i < endsamp; i += inputchans)
         rms_gauge->tick(in[i + inchan]);
      pk = rms_gauge->lastOut();
   }
   else if (detector_type == PEAK_DETECTOR) {
      for (int i = offset; i < endsamp; i += inputchans) {
         float samp = in[i + inchan];
         if (samp < 0.0)
            samp = -samp;
         if (samp > pk)
            pk = samp;
      }
   }
   else { /* detector_type == AVERAGE_DETECTOR */
      double dpk = 0.0;
      for (int i = offset; i < endsamp; i += inputchans) {
         double samp = (double) in[i + inchan];
         if (samp < 0.0)
            samp = -samp;
         dpk += samp;
      }
      pk = (float) (dpk / (double) window_len);
   }

#ifdef DEBUG
   printf("\npeak: %f\n", pk);
#endif

   return pk;
}


inline float COMPLIMIT::get_compress_factor()
{
   float diff, target_level, factor;

   diff = peak - threshold;
   target_level = threshold + (diff / ratio);
   factor = target_level / peak;

#ifdef DEBUG
   printf("scale factor: %f\n", factor);
#endif

   return factor;
}


int COMPLIMIT::run()
{
   int   i, samps, count, end_count;
   float sig, out[2];

   samps = chunksamps * inputchans;

   rtgetin(in, this, samps);

   end_count = window_len - 1;
   count = end_count;                   /* trigger first look-ahead */

   for (i = 0; i < samps; i += inputchans) {
      if (--branch < 0) {
         if (amptable)
            oamp = tablei(cursamp, amptable, amptabs);
         branch = skip;
      }

      if (count == end_count) {
         prev_peak = peak;
         peak = get_peak(i) * ingain;      /* get peak for upcoming window */
         if (peak > threshold) {
            if (env_stage == ENV_INACTIVE) {
               target_scale = get_compress_factor();
               scale_increment = (1.0 - target_scale) / (float) atk_samps;
               scale = 1.0;
               env_count = 0;
               env_stage = ENV_ATTACK;
            }
            else {
               if (env_stage == ENV_RELEASE) {
                  target_scale = get_compress_factor();
/*
                  if (scale > target_scale)
                  else
*/
                  scale_increment = (scale - target_scale) / (float) atk_samps;
                  env_count = 0;
                  env_stage = ENV_ATTACK;
               }
               else if (peak > prev_peak) {
                  target_scale = get_compress_factor();
                  scale_increment = (scale - target_scale) / (float) atk_samps;
                  env_count = 0;
                  env_stage = ENV_ATTACK;
               }
            }
            peak_under_thresh = 0;
         }
         else
            peak_under_thresh = 1;
         count = 0;
      }
      else
         count++;

      if (bypass)
         sig = in[i + inchan] * oamp;
      else {
         sig = in[i + inchan] * ingain;

         if (env_stage == ENV_ATTACK) {
#ifdef DEBUG
            printf("in attack:   cursamp=%d, env_count=%d, insig=% f, scale=%f",
                                               cursamp, env_count, sig, scale);
#endif
            sig *= scale;

            if (env_count == atk_samps) {
               scale = target_scale;
               env_stage = ENV_SUSTAIN;
            }
            else {
               scale -= scale_increment;
               env_count++;
            }
         }
         else if (env_stage == ENV_SUSTAIN) {
#ifdef DEBUG
            printf("in sustain:  cursamp=%d, insig=% f, scale=%f",
                                                         cursamp, sig, scale);
#endif
            sig *= scale;

            if (peak_under_thresh) {
               scale_increment = (1.0 - scale) / (float) rel_samps;
               env_count = 0;
               env_stage = ENV_RELEASE;
            }
         }
         else if (env_stage == ENV_RELEASE) {
#ifdef DEBUG
            printf("in release:  cursamp=%d, env_count=%d, insig=% f, scale=%f",
                                               cursamp, env_count, sig, scale);
#endif
            sig *= scale;
            scale += scale_increment;

            if (env_count == rel_samps) {
               env_count = 0;
               env_stage = ENV_INACTIVE;
            }
            else
               env_count++;
         }
#ifdef DEBUG
         if (env_stage != ENV_INACTIVE)
            printf(", outsig=% f\n", sig);
#endif
         sig *= outgain * oamp;
      }

      if (NCHANS == 2) {
         out[0] = sig * pctleft;
         out[1] = sig * (1.0 - pctleft);
      }
      else
         out[0] = sig;

      rtaddout(out);
      cursamp++;
   }

   return i;
}


Instrument *makeCOMPLIMIT()
{
   COMPLIMIT *inst;

   inst = new COMPLIMIT();
   return inst;
}


void rtprofile()
{
   RT_INTRO("COMPLIMIT", makeCOMPLIMIT);
}

