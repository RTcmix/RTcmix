/* COMPLIMIT: a compressor-limiter

   p0  = output start time
   p1  = input start time
   p2  = duration
   p3  = gain applied to input before compression (in dB)
   p4  = gain applied to output after compression - "makeup gain" (in dB)
   p5  = attack time (seconds)
   p6  = release time (seconds)
   p7  = threshold (in dBFS)
   p8  = compression ratio - e.g. 20 means 20:1 (100 is infinity)
   p9  = look-ahead time (seconds)
   p10 = peak detection window size (power of 2 <= RTCmix output buffer size)
   p11 = detection type (0: peak, 1: average peak, 2: rms)
   p12 = bypass (1: bypass on, 0: bypass off)
   p13 = input channel  [optional; default is 0]
   p14 = percent output to left channel (0 - 1)  [optional; default is 0.5]

   BUGS:
      - Sometimes the compressor will stay in sustain for too long.
        This happens when the source sound has a long decaying envelope
        where some of the decay is above the threshold.

   John Gibson <johngibson@virginia.edu>,  21 April, 2000
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>       /* for DBL_MAX */
#include <mixerr.h>
#include <Instrument.h>
#include "COMPLIMIT.h"
#include <rt.h>
#include <rtdefs.h>
#include <assert.h>

//#define DEBUG

#ifdef DEBUG
   #define dprint(msg)        printf((msg))
   #define dprint1(msg, arg)  printf((msg), (arg))
#else
   #define dprint(msg)
   #define dprint1(msg, arg)
#endif

/* experiment to relieve symptom of bug described in comment above
   (major problem for pnoEb2stop.tm2.aif)
*/
#define LONG_SUSTAIN_FIX
#define MAX_WINS 5        /* just a guess */

#define DEFAULT_WINDOW_SIZE   128

extern "C" {
   #include <ugens.h>
   extern int resetval;
}


COMPLIMIT::COMPLIMIT() : Instrument()
{
   in = NULL;
   first_time = 1;
   env_state = ENV_INACTIVE;
   env_count = -1;
   sus_count = 0;
   target_peak = 0.0;
   next_target_gain = 0.0;
   wins_under_thresh = 0;
   branch = 0;
   oamp = 1.0;                  /* in case amptable == NULL */
}


COMPLIMIT::~COMPLIMIT()
{
   delete [] in;
}


inline int min(int a, int b) {
   if (a < b)
      return a;
   else
      return b;
}


int COMPLIMIT::init(float p[], short n_args)
{
   int   detector_int;
   float outskip, inskip, dur, atk_time, rel_time, lookahead_time;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   ingain = ampdb(p[3]);           /* in dB, 0 means no change */
   outgain = ampdb(p[4]);          /* in dB */
   atk_time = p[5];
   rel_time = p[6];
   threshold = p[7];               /* in dBFS */
   ratio = p[8];
   lookahead_time = p[9];
   window_frames = (int) p[10];
   detector_int = (int) p[11];
   bypass = (int) p[12];
   inchan = (n_args > 13) ? (int) p[13] : 0;
   pctleft = (n_args > 14) ? p[14] : 0.5;

   rtsetinput(inskip, this);
   nsamps = rtsetoutput(outskip, dur, this);

   if (inchan >= inputchans) {
      fprintf(stderr, "You asked for channel %d of a %d-channel file.\n",
                      inchan, inputchans);
      exit(1);
   }
   if (outputchans > 2) {
      fprintf(stderr, "Can't use more than 2 output channels.\n");
      exit(1);
   }

   if (atk_time < 0.0 || rel_time < 0.0) {
      fprintf(stderr, "Invalid envelope times.\n");
      exit(1);
   }
   atk_samps = (int) (atk_time * SR + 0.5);
   rel_samps = (int) (rel_time * SR + 0.5);

   lookahead_samps = (int) (lookahead_time * SR + 0.5);

   dbref = dbamp(32768.0);           // FIXME: but what about 24-bit output?

   if (threshold < -dbref || threshold > 0.0) {
      fprintf(stderr, "Threshold must be between %.2f and 0.\n", -dbref);
      exit(1);
   }
   threshold = ampdb(threshold + dbref);

   if (ratio < 1.0) {
      fprintf(stderr, "Compression ratio must be 1 or greater.\n");
      exit(1);
   }
   if (ratio >= 100.0)
      ratio = DBL_MAX;

   if (window_frames == 0) {
      window_frames = min(DEFAULT_WINDOW_SIZE, RTBUFSAMPS);
      printf("Setting window size to %d frames.\n", window_frames);
   }
   else if (window_frames > RTBUFSAMPS) {
      fprintf(stderr, "Window size must be less than %d frames (the RTCmix "
                      "buffer size). Correcting...\n", RTBUFSAMPS);
      window_frames = RTBUFSAMPS;
   }
   else if (RTBUFSAMPS % window_frames) {
      fprintf(stderr, "Window size must be a power of two.\n");
      exit(1);
   }

   if (detector_int < 0 || detector_int > 2) {
      fprintf(stderr, "Invalid detector type.\n");
      exit(1);
   }
   detector_type = (DetectType) detector_int;

   amptable = floc(1);
   if (amptable) {
      int amplen = fsize(1);
      tableset(dur, amplen, amptabs);
   }
   else
      printf("Setting phrase curve to all 1's\n");

   skip = (int) (SR / (float) resetval);

#ifdef DEBUG
   printf("ingain: %g, outgain: %g, threshold: %g, ratio: %g:1, atk_samps: %d,"
          " rel_samps: %d, lookahead_samps: %d, window_frames: %d\n",
          ingain, outgain, threshold, ratio, atk_samps, rel_samps,
          lookahead_samps, window_frames);
#endif

   return nsamps;
}


/* Return the peak of a segment of the input buffer. The segment starts at
   <offset> samples (not frames) and contains <window_len> frames worth. The
   peak detection method is either absolute peak, average peak or RMS.
   If detecting absolute peak, pass back the *frame* offset of the first
   sample that exceeds the threshold, relative to <offset>. Note that this
   sample may not be the peak for the window. If detecting peak by either of
   the two averaging methods, pass pack zero.
*/
inline float COMPLIMIT::get_peak(int offset, int *over_thresh_offset)
{
   int   endsamp;
   float pk = 0.0;
#ifdef DEBUG
   float over = 0.0;
#endif

   endsamp = offset + (window_len * inputchans);
   *over_thresh_offset = 0;

   if (detector_type == PEAK_DETECTOR) {
      int loc = -1;
      for (int i = offset; i < endsamp; i += inputchans) {
         float samp = in[i + inchan];
         if (samp < 0.0)
            samp = -samp;
         if (loc == -1 && samp > threshold) {
            loc = i - offset;
#ifdef DEBUG
            over = samp;
#endif
         }
         if (samp > pk)
            pk = samp;
      }
      *over_thresh_offset = loc / inputchans;
   }
   else if (detector_type == AVERAGE_DETECTOR) {
      double dpk = 0.0;
      for (int i = offset; i < endsamp; i += inputchans) {
         double samp = (double) in[i + inchan];
         if (samp < 0.0)
            samp = -samp;
         dpk += samp;
      }
      pk = (float) (dpk / (double) window_len);
   }
   else {  /* detector_type == RMS_DETECTOR */
      double dpk = 0.0;
      for (int i = offset; i < endsamp; i += inputchans) {
         double samp = (double) in[i + inchan];
         dpk += samp * samp;
      }
      pk = (float) sqrt(dpk / (double) window_len);
   }

#ifdef DEBUG
   printf("\npeak: %g, over: %g, over_thresh_offset: %d, firstsamp: %d, "
          "window_len: %d, offset: %d\n", pk, over, *over_thresh_offset,
                         cursamp + lookahead_samps, window_len, offset);
#endif

   return pk;
}


inline float COMPLIMIT::get_gain_reduction()
{
   float  gain_reduction;
   double peak_db, threshold_db, level_db, diff;

   peak_db = dbamp(target_peak);
   threshold_db = dbamp(threshold);      // FIXME: pre-calculate this

   diff = peak_db - threshold_db;
   level_db = threshold_db + (diff / ratio);
   gain_reduction = (float) (level_db - peak_db);

   dprint1("gain reduction: %f\n", gain_reduction);

   return gain_reduction;
}


int COMPLIMIT::run()
{
   int   i, samps, win_count, win_end_count, over_thresh_offset;
   float *bufendptr, sig, out[2];

   Instrument::run();

   samps = chunksamps * inputchans;

   if (first_time) {
      int nbufs = (lookahead_samps / RTBUFSAMPS) + 2;
      int extrasamps = RTBUFSAMPS - chunksamps;
      if (extrasamps)
         nbufs++;
      buf_samps = nbufs * RTBUFSAMPS * inputchans;
      in = new float [buf_samps];
      for (i = 0; i < buf_samps; i++)
         in[i] = 0.0;

      /* Note: input ptr chases read ptr by lookahead_samps */
      inptr = in + (buf_samps - (lookahead_samps * inputchans));
      bufstartptr = in + (extrasamps * inputchans);
      readptr = bufstartptr;

      offset = extrasamps * inputchans;
      window_len = chunksamps % window_frames;
      if (window_len == 0)
         window_len = window_frames;
   }

   bufendptr = in + buf_samps;

   win_end_count = window_len - 1;
   win_count = 0;                      /* trigger first look-ahead */

#ifdef DEBUG
   printf("\nin=%p, inptr=%p, readptr=%p, bufstartptr=%p, bufendptr=%p\n",
                                 in, inptr, readptr, bufstartptr, bufendptr);
#endif
   rtgetin(readptr, this, samps);

   for (i = 0; i < chunksamps; i++) {
      if (--branch < 0) {
         if (amptable)
            oamp = tablei(cursamp, amptable, amptabs);
         branch = skip;
      }

      if (win_count == 0) {
         float peak = get_peak(offset, &over_thresh_offset);

         if (peak > threshold) {
            dprint1("current increment: %f\n", gain_increment);
            switch (env_state) {
               case ENV_INACTIVE:
                  target_peak = peak;
                  target_gain = get_gain_reduction();
                  gain_increment = target_gain / (float) atk_samps;
                  gain = 0.0;                   /* in dB */
                  env_count = atk_samps;
                  sus_count = window_len - (over_thresh_offset + 1);
                  env_state = ENV_ATTACK_WAIT;
                  break;
               case ENV_ATTACK:
                  if (peak > target_peak) {      /* re-start attack */
                     target_peak = peak;
                     float tmpgain = get_gain_reduction();
                     float tmpincr = (tmpgain - gain)
                                    / (float) (atk_samps + over_thresh_offset);
                     if (next_target_gain) {    /* new gain already pending */
                        next_target_gain = 0;   /* override it */
                        dprint("W ATTACK: OVERRIDE PENDING GAIN...\n");
                     }
                     if (tmpincr < gain_increment) {   /* NB: both negative */
                        target_gain = tmpgain;
                        gain_increment = tmpincr;
                        env_count = atk_samps + over_thresh_offset;
                        sus_count = window_len - (over_thresh_offset + 1);
                        dprint("W ATTACK: RESTART ATTACK\n");
                     }
                     else {
                        /* delay new ramp until current target_gain reached */
                        next_target_gain = tmpgain;
                        next_env_count = (atk_samps + over_thresh_offset)
                                                                   - env_count;
                        dprint("W ATTACK: NEW GAIN PENDING...\n");
                     }
                  }
                  else                          /* stay in sustain for longer */
                     sus_count += window_len * (wins_under_thresh + 1);
                  break;
               case ENV_SUSTAIN:
                  if (peak > target_peak) {     /* re-start attack */
                     target_peak = peak;
                     target_gain = get_gain_reduction();
                     gain_increment = (target_gain - gain)
                                    / (float) (atk_samps + over_thresh_offset);
                     env_count = atk_samps + over_thresh_offset;
                     sus_count = window_len - (over_thresh_offset + 1);
                     env_state = ENV_ATTACK;
                     dprint("W SUSTAIN -> RESTART ATTACK\n");
                  }
                  else {                        /* stay in sustain for longer */
#ifdef LONG_SUSTAIN_FIX
                     if (wins_under_thresh < MAX_WINS)
                        sus_count += window_len * (wins_under_thresh + 1);
#else
                     sus_count += window_len * (wins_under_thresh + 1);
#endif
                  }
                  break;
               case ENV_RELEASE:
                  target_peak = peak;
                  target_gain = get_gain_reduction();
                  gain_increment = (target_gain - gain)
                                    / (float) (atk_samps + over_thresh_offset);
                  env_count = atk_samps + over_thresh_offset;
                  sus_count = window_len - (over_thresh_offset + 1);
                  env_state = ENV_ATTACK;
                  break;
               case ENV_ATTACK_WAIT:            /* should never get here */
                  assert(env_state != ENV_ATTACK_WAIT);
                  break;
            }
            dprint1("new increment: %f\n", gain_increment);
            wins_under_thresh = 0;
         }  /* if (peak > threshold) */
         else
            wins_under_thresh++;

         offset += window_len * inputchans;
         if (offset >= buf_samps)
            offset = 0;
         if (window_len != window_frames) {     /* after first window */
            window_len = window_frames;
            win_count = win_end_count;
            win_end_count = window_len - 1;
         }
         else
            win_count = win_end_count;
#ifdef DEBUG
         printf("sus_count=%d, wins_under_thresh=%d\n",
                                                sus_count, wins_under_thresh);
#endif
      }
      else
         win_count--;

      if (env_state == ENV_ATTACK_WAIT) {
         if (over_thresh_offset == 0)
            env_state = ENV_ATTACK;  /* timer elapsed; start attack this samp */
         else
            over_thresh_offset--;
      }

      sig = inptr[inchan] * ingain;

      if (env_state == ENV_ATTACK) {
         double scale = ampdb(gain);
#ifdef DEBUG
         printf("attack:   cursamp=%d, envcount=%d, insig=% f, gain=%f, sc=%f",
                                       cursamp, env_count, sig, gain, scale);
#endif
         sig *= scale;

         if (env_count == 0) {
            if (next_target_gain) {
               gain = target_gain;
               target_gain = next_target_gain;
               next_target_gain = 0;
               env_count = next_env_count - 1;
               gain_increment = (target_gain - gain) / (float) env_count;
            }
            else {
               gain = target_gain;
               env_state = ENV_SUSTAIN;
            }
         }
         else {
            gain += gain_increment;
            env_count--;
         }
      }
      else if (env_state == ENV_SUSTAIN) {
         /* not efficient to keep converting from the same gain, but... */
         double scale = ampdb(gain);
#ifdef DEBUG
         printf("sustain:  cursamp=%d, envcount=%d, insig=% f, gain=%f, sc=%f",
                                       cursamp, env_count, sig, gain, scale);
#endif
         sig *= scale;

         if (sus_count == 0) {
            gain_increment = gain / (float) rel_samps;
            env_count = rel_samps;
            env_state = ENV_RELEASE;
         }
         else
            sus_count--;
      }
      else if (env_state == ENV_RELEASE) {
         double scale = ampdb(gain);
#ifdef DEBUG
         printf("release:  cursamp=%d, envcount=%d, insig=% f, gain=%f, sc=%f",
                                       cursamp, env_count, sig, gain, scale);
#endif
         sig *= scale;
         gain -= gain_increment;

         if (env_count == 0) {
            env_count = -1;
            env_state = ENV_INACTIVE;
         }
         else
            env_count--;
      }
#ifdef DEBUG
      if (env_state != ENV_INACTIVE)
         printf(", outsig=% f\n", sig);
      if (fabs(sig) > threshold)
         printf("OUTPUT EXCEEDS THRESHOLD! (%f, %f)\n", sig, threshold);
#endif
      sig *= outgain * oamp;

      if (bypass)
         sig = inptr[inchan] * oamp;

      if (outputchans == 2) {
         out[0] = sig * pctleft;
         out[1] = sig * (1.0 - pctleft);
      }
      else
         out[0] = sig;

      rtaddout(out);
      cursamp++;

      inptr += inputchans;
      if (inptr >= bufendptr) {
         inptr = bufstartptr;
         bufstartptr = in;
         dprint1("    resetting inptr=%p\n", inptr);
      }
   }
   readptr += samps;
   if (readptr >= bufendptr)
      readptr = in;

   if (first_time)
      first_time = 0;

   return i;
}


Instrument *makeCOMPLIMIT()
{
   COMPLIMIT *inst;

   inst = new COMPLIMIT();
   inst->set_bus_config("COMPLIMIT");

   return inst;
}


void rtprofile()
{
   RT_INTRO("COMPLIMIT", makeCOMPLIMIT);
}

