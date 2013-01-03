/* COMPLIMIT - a compressor-limiter, with lookahead

   The parameters marked with '*' can receive dynamic updates from a table
   or a real-time control source.

      p0  = output start time
      p1  = input start time
      p2  = duration
    * p3  = gain applied to input before compression (in dBFS; 0 is no change)
    * p4  = gain applied to output after compression - "makeup gain" (in dBFS)
    * p5  = attack time (seconds)
    * p6  = release time (seconds)
    * p7  = threshold (in dBFS)
    * p8  = compression ratio - e.g. 20 means 20:1 (100 is infinity)
      p9  = look-ahead time (seconds)
      p10 = peak detection window size (power of 2 <= RTCmix output buffer size)
    * p11 = detection type (0: peak, 1: average peak, 2: rms)
            [optional; default is 0)
    * p12 = bypass (1: bypass on, 0: bypass off) [optional; default is 0]
    * p13 = input channel  [optional; default is 0]
    * p14 = percent output to left channel (0 - 1)  [optional; default is 0.5]

   NOTES

   1. For backward compatibility with pre-v4 scores, the optional gen table 1
      scales the makeup gain (p4), after conversion from dB to linear amp.
      When bypass (p12) is on, there is no enveloping at all in this version.

   2. BUG: Sometimes the compressor will stay in sustain for too long.  This
      happens when the source sound has a long decaying envelope during which
      some of the decay is above the threshold.


   John Gibson <johgibso at indiana dot edu>, 4/21/00; rev. for v4, 6/18/05
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>       // DBL_MAX
#include <ugens.h>
#include <Instrument.h>
#include "COMPLIMIT.h"
#include <rt.h>
#include <rtdefs.h>
#include <assert.h>

//#define DEBUG
//#define DEBUG2

#ifdef DEBUG
   #define DPRINT(msg)        printf((msg))
   #define DPRINT1(msg, arg)  printf((msg), (arg))
#else
   #define DPRINT(msg)
   #define DPRINT1(msg, arg)
#endif

// experiment to relieve symptom of bug described in comment above
// (major problem for pnoEb2stop.tm2.aif)
#define LONG_SUSTAIN_FIX
#define MAX_WINS 5        // just a guess

#define DEFAULT_WINDOW_SIZE   128


COMPLIMIT::COMPLIMIT() : Instrument()
{
   in = NULL;
   first_time = true;
   env_state = ENV_INACTIVE;
   env_count = -1;
   sus_count = 0;
   gain = 0.0;
   ingain = -DBL_MAX;
   outgain = -DBL_MAX;
   threshold_dbfs = -DBL_MAX;
   atk_time = -DBL_MAX;
   rel_time = -DBL_MAX;
   target_peak = 0.0;
   next_target_gain = 0.0;
   wins_under_thresh = 0;
   branch = 0;
}

COMPLIMIT::~COMPLIMIT()
{
   delete [] in;
}

int COMPLIMIT::usage()
{
	return die("COMPLIMIT", "Usage: COMPLIMIT(start, inskip, dur, ingain, "
              "outgain, atktime, reltime, threshold, ratio, lookahead, "
              "[windowsize, detection_type, bypass, inchan, pan])");
}

inline int min(int a, int b) { return (a < b) ? a : b; }

DetectType COMPLIMIT::getDetectType(double pval)
{
   int intval = int(pval);
   DetectType type = PEAK_DETECTOR;

   switch (intval) {
      case 0:
         type = PEAK_DETECTOR;
         break;
      case 1:
         type = AVERAGE_DETECTOR;
         break;
      case 2:
         type = RMS_DETECTOR;
         break;
      default:
         die("COMPLIMIT", "Invalid detector type %d\n.", intval);
         break;
   }
   return type;
}

int COMPLIMIT::init(double p[], int n_args)
{
   nargs = n_args;
   if (nargs < 11)
      return usage();

   const float outskip = p[0];
   const float inskip = p[1];
   const float dur = p[2];

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;

   inchan = int(p[13]);
   if (inchan >= inputChannels())
      return die("COMPLIMIT", "You asked for channel %d of a %d-channel input.",
                                                     inchan, inputChannels());
   if (outputChannels() > 2)
      return die("COMPLIMIT", "Can't use more than 2 output channels.");

   const float lookahead_time = p[9];
   lookahead_samps = int(lookahead_time * SR + 0.5);

   dbref = dbamp(32768.0);
   // Verify initial value of threshold, in dbFS.
   if (p[7] < -dbref || p[7] > 0.0)
      return die("COMPLIMIT", "Threshold must be between %.2f and 0.", -dbref);

   // Verify initial value of ratio.
   ratio = p[8];
   if (ratio < 1.0)
      return die("COMPLIMIT", "Compression ratio must be 1 or greater.");
   if (ratio >= 100.0)
      ratio = DBL_MAX;

   window_frames = int(p[10]);
   if (window_frames == 0) {
      window_frames = min(DEFAULT_WINDOW_SIZE, RTBUFSAMPS);
      rtcmix_advise("COMPLIMIT", "Setting window size to %d frames.", window_frames);
   }
   else if (window_frames > RTBUFSAMPS) {
      rtcmix_warn("COMPLIMIT", "Window size must be <= the RTcmix buffer size "
                        "(currently %d frames).  Correcting...", RTBUFSAMPS);
      window_frames = RTBUFSAMPS;
   }
   else if (RTBUFSAMPS % window_frames)
      return die("COMPLIMIT", "RTcmix buffer size must be a multiple of the "
                              "COMPLIMIT window size.");

   detector_type = getDetectType(p[11]);

   // for backward compatibility with pre-v4 scores
   amptable = floc(1);
   if (amptable) {
      int amplen = fsize(1);
      tableset(SR, dur, amplen, amptabs);
   }

#ifdef DEBUG
   printf("lookahead_samps: %d, window_frames: %d\n",
          lookahead_samps, window_frames);
#endif

   return nSamps();
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
   float pk = 0.0;
#ifdef DEBUG
   float over = 0.0;
#endif

   const int inchans = inputChannels();
   const int endsamp = offset + (window_len * inchans);

   *over_thresh_offset = 0;

   if (detector_type == PEAK_DETECTOR) {
      int loc = -1;
      for (int i = offset; i < endsamp; i += inchans) {
         float samp = in[i + inchan];
         if (samp < 0.0)
            samp = -samp;
         if (loc == -1 && samp > threshold_amp) {
            loc = i - offset;
#ifdef DEBUG
            over = samp;
#endif
         }
         if (samp > pk)
            pk = samp;
      }
      *over_thresh_offset = loc / inchans;
   }
   else if (detector_type == AVERAGE_DETECTOR) {
      double dpk = 0.0;
      for (int i = offset; i < endsamp; i += inchans) {
         double samp = in[i + inchan];
         if (samp < 0.0)
            samp = -samp;
         dpk += samp;
      }
      pk = dpk / window_len;
   }
   else {  // detector_type == RMS_DETECTOR
      double dpk = 0.0;
      for (int i = offset; i < endsamp; i += inchans) {
         double samp = in[i + inchan];
         dpk += samp * samp;
      }
      pk = sqrt(dpk / window_len);
   }

#ifdef DEBUG2
   printf("\npeak: %g, over: %g, over_thresh_offset: %d, firstsamp: %d, "
          "window_len: %d, offset: %d\n", pk, over, *over_thresh_offset,
                         currentFrame() + lookahead_samps, window_len, offset);
#endif

   return pk;
}


inline float COMPLIMIT::get_gain_reduction()
{
   const double peak_db = dbamp(target_peak);
   const double diff = peak_db - threshold_db;
   const double level_db = threshold_db + (diff / ratio);
   const float gain_reduction = float(level_db - peak_db);

#ifdef DEBUG
   printf("get_gain_reduction(): target=%g, thresh=%g dB, level=%g dB, "
         "reduction=%g\n", target_peak, threshold_db, level_db, gain_reduction);
#endif

   return gain_reduction;
}


int COMPLIMIT::configure()
{
   return 0;
}


void COMPLIMIT::doupdate()
{
   double p[15];
   update(p, 15, 1 << 3 | 1 << 4 | 1 << 5 | 1 << 6 | 1 << 7 | 1 << 8
                                 | 1 << 11 | 1 << 12 | 1 << 13 | 1 << 14);

   if (ingain != p[3]) {
      ingain = p[3];
      inamp = ampdb(ingain);
   }
   if (outgain != p[4]) {
      outgain = p[4];
      outamp = ampdb(outgain);
   }
   if (amptable)  // backwards compatibility
      outamp *= tablei(currentFrame(), amptable, amptabs);

   if (atk_time != p[5]) {
      atk_time = p[5];
      atk_samps = (atk_time < 0.0) ? 0 : int(atk_time * SR + 0.5);
   }
   if (rel_time != p[6]) {
      rel_time = p[6];
      rel_samps = (rel_time < 0.0) ? 0 : int(rel_time * SR + 0.5);
   }

   if (threshold_dbfs != p[7]) {
      threshold_dbfs = p[7];
      if (threshold_dbfs < -dbref)
         threshold_dbfs = -dbref;
      else if (threshold_dbfs > 0.0)
         threshold_dbfs = 0;
      // pre-calculate for get_gain_reduction()...
      threshold_db = threshold_dbfs + dbref;
      threshold_amp = ampdb(threshold_db);
   }

   ratio = p[8];
   if (ratio < 1.0)
      ratio = 1.0;
   else if (ratio >= 100.0)
      ratio = DBL_MAX;

   detector_type = getDetectType(p[11]);

   bypass = bool(p[12]);
   inchan = int(p[13]);
   pan = (nargs > 14) ? p[14] : 0.5f;

#ifdef DEBUG
   printf("inamp: %g, outamp: %g, threshold: %g (amp), ratio: %g:1, "
          "atk_samps: %d, rel_samps: %d\n",
          inamp, outamp, threshold_amp, ratio, atk_samps, rel_samps);
#endif
}


int COMPLIMIT::run()
{
   const int nframes = framesToRun();
   const int inchans = inputChannels();
   const int samps = nframes * inchans;

   if (first_time) {
      int nbufs = (lookahead_samps / RTBUFSAMPS) + 2;
      int extrasamps = RTBUFSAMPS - nframes;
      if (extrasamps)
         nbufs++;
      buf_samps = nbufs * RTBUFSAMPS * inchans;
      in = new float [buf_samps];
      for (int i = 0; i < buf_samps; i++)
         in[i] = 0.0;

      // Note: input ptr chases read ptr by lookahead_samps
      inptr = in + (buf_samps - (lookahead_samps * inchans));
      bufstartptr = in + (extrasamps * inchans);
      readptr = bufstartptr;

      offset = extrasamps * inchans;
      window_len = nframes % window_frames;
      if (window_len == 0)
         window_len = window_frames;
   }

   float *bufendptr = in + buf_samps;

   int win_end_count = window_len - 1;
   int win_count = 0;                  // trigger first look-ahead

#ifdef DEBUG
   printf("\nin=%p, inptr=%p, readptr=%p, bufstartptr=%p, bufendptr=%p\n",
                                 in, inptr, readptr, bufstartptr, bufendptr);
#endif
   rtgetin(readptr, this, samps);

   int over_thresh_offset = 0;

   for (int i = 0; i < nframes; i++) {
      if (--branch <= 0) {
         doupdate();
         branch = getSkip();
      }

      if (win_count == 0) {
         float peak = get_peak(offset, &over_thresh_offset);

         if (peak > threshold_amp) {
            DPRINT1("current increment: %f\n", gain_increment);
            switch (env_state) {
               case ENV_INACTIVE:
                  target_peak = peak;
                  target_gain = get_gain_reduction();
                  gain_increment = target_gain / float(atk_samps);
                  gain = 0.0;                   // in dB
                  env_count = atk_samps;
                  sus_count = window_len - (over_thresh_offset + 1);
                  env_state = ENV_ATTACK_WAIT;
                  break;
               case ENV_ATTACK:
                  if (peak > target_peak) {      // re-start attack
                     target_peak = peak;
                     float tmpgain = get_gain_reduction();
                     float tmpincr = (tmpgain - gain)
                                    / float(atk_samps + over_thresh_offset);
                     if (next_target_gain) {    // new gain already pending
                        next_target_gain = 0;   // override it
                        DPRINT("W ATTACK: OVERRIDE PENDING GAIN...\n");
                     }
                     if (tmpincr < gain_increment) {   // NB: both negative
                        target_gain = tmpgain;
                        gain_increment = tmpincr;
                        env_count = atk_samps + over_thresh_offset;
                        sus_count = window_len - (over_thresh_offset + 1);
                        DPRINT("W ATTACK: RESTART ATTACK\n");
                     }
                     else {
                        // delay new ramp until current target_gain reached
                        next_target_gain = tmpgain;
                        next_env_count = (atk_samps + over_thresh_offset)
                                                                   - env_count;
                        DPRINT("W ATTACK: NEW GAIN PENDING...\n");
                     }
                  }
                  else                          // stay in sustain for longer
                     sus_count += window_len * (wins_under_thresh + 1);
                  break;
               case ENV_SUSTAIN:
                  if (peak > target_peak) {     // re-start attack
                     target_peak = peak;
                     target_gain = get_gain_reduction();
                     gain_increment = (target_gain - gain)
                                    / float(atk_samps + over_thresh_offset);
                     env_count = atk_samps + over_thresh_offset;
                     sus_count = window_len - (over_thresh_offset + 1);
                     env_state = ENV_ATTACK;
                     DPRINT("W SUSTAIN -> RESTART ATTACK\n");
                  }
                  else {                        // stay in sustain for longer
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
                                    / float(atk_samps + over_thresh_offset);
                  env_count = atk_samps + over_thresh_offset;
                  sus_count = window_len - (over_thresh_offset + 1);
                  env_state = ENV_ATTACK;
                  break;
               case ENV_ATTACK_WAIT:            // should never get here
                  assert(env_state != ENV_ATTACK_WAIT);
                  break;
            }
            DPRINT1("new increment: %f\n", gain_increment);
            wins_under_thresh = 0;
         }  // if (peak > threshold_amp)
         else
            wins_under_thresh++;

         offset += window_len * inchans;
         if (offset >= buf_samps)
            offset = 0;
         if (window_len != window_frames) {     // after first window
            window_len = window_frames;
            win_count = win_end_count;
            win_end_count = window_len - 1;
         }
         else
            win_count = win_end_count;
#ifdef DEBUG2
         printf("sus_count=%d, wins_under_thresh=%d\n",
                                                sus_count, wins_under_thresh);
#endif
      }
      else
         win_count--;

      if (env_state == ENV_ATTACK_WAIT) {
         if (over_thresh_offset == 0)
            env_state = ENV_ATTACK;  // timer elapsed; start attack this samp
         else
            over_thresh_offset--;
      }

      float sig = inptr[inchan] * inamp;

      if (env_state == ENV_ATTACK) {
         const double scale = ampdb(gain);
#ifdef DEBUG2
         printf("attack:   cursamp=%d, envcount=%d, insig=% f, gain=%f, sc=%f",
                                 currentFrame(), env_count, sig, gain, scale);
#endif
         sig *= scale;

         if (env_count == 0) {
            if (next_target_gain) {
               gain = target_gain;
               target_gain = next_target_gain;
               next_target_gain = 0;
               env_count = next_env_count - 1;
               gain_increment = (target_gain - gain) / float(env_count);
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
         // not efficient to keep converting from the same gain, but...
         double scale = ampdb(gain);
#ifdef DEBUG2
         printf("sustain:  cursamp=%d, envcount=%d, insig=% f, gain=%f, sc=%f",
                                 currentFrame(), env_count, sig, gain, scale);
#endif
         sig *= scale;

         if (sus_count == 0) {
            gain_increment = gain / float(rel_samps);
            env_count = rel_samps;
            env_state = ENV_RELEASE;
         }
         else
            sus_count--;
      }
      else if (env_state == ENV_RELEASE) {
         double scale = ampdb(gain);
#ifdef DEBUG2
         printf("release:  cursamp=%d, envcount=%d, insig=% f, gain=%f, sc=%f",
                                 currentFrame(), env_count, sig, gain, scale);
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
#ifdef DEBUG2
      if (env_state != ENV_INACTIVE)
         printf(", outsig=% f\n", sig);
      if (fabs(sig) > threshold_amp)
         printf("OUTPUT EXCEEDS THRESHOLD! (%f, %f)\n", sig, threshold_amp);
#endif
      sig *= outamp;

      if (bypass)
         sig = inptr[inchan];

      float out[2];
      if (outputChannels() == 2) {
         out[0] = sig * pan;
         out[1] = sig * (1.0f - pan);
      }
      else
         out[0] = sig;

      rtaddout(out);
      increment();

      inptr += inchans;
      if (inptr >= bufendptr) {
         inptr = bufstartptr;
         bufstartptr = in;
         DPRINT1("    resetting inptr=%p\n", inptr);
      }
   }
   readptr += samps;
   if (readptr >= bufendptr)
      readptr = in;

   if (first_time)
      first_time = false;

   return nframes;
}


Instrument *makeCOMPLIMIT()
{
   COMPLIMIT *inst;

   inst = new COMPLIMIT();
   inst->set_bus_config("COMPLIMIT");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("COMPLIMIT", makeCOMPLIMIT);
}
#endif
