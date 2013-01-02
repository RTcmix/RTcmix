/* GRANULATE - granulation of sound stored in a table

   WARNING: This is my first granulation instrument. It has a bug that
   results in an unintended dependency between traversal rate, grain rate,
   and output jitter. The fix is included in GRANULATE2, which also has
   several new features.

   Any parameter marked with '*' can receive updates from a real-time
   control source.

      p0  = output start time
    * p1  = input start time (will be constrained to window, p5-6)
            NOTE: Unlike other instruments, this applies to a table, not to
            an input sound file.
      p2  = total duration
    * p3  = amplitude multiplier

      p4  = input sound table (e.g., maketable("soundfile", ...))
      p5  = number of channels in sample table
    * p6  = input channel of sample table

    * p7  = input window start time
    * p8  = input window end time
    * p9  = wraparound (see below)

      The "input window" refers to the portion of the input sound table read
      by the granulator.  When the granulator reaches the end of the window,
      it wraps around to the beginning, unless p9 (wraparound) is false (0).
      In that case, please note that any data coming from time-varying tables
      (such as p3), may not span the entire table.

      It's best to arrange for the window start to be a little after the sound
      table start, and for the window end to be a little before the table end.

    * p10 = traversal rate

      The granulator moves through the input window at the "traversal rate."
      Here are some sample values:

            0     no movement
            1     move forward at normal rate
            2.5   move forward at a rate that is 2.5 times normal
            -1    move backward at normal rate

      The following parameters determine the character of individual grains.

      p11 = grain envelope table

    * p12 = grain hop time (time between successive grains).  This is the 
            inverse of grain density (grains per second); you can use
            "hoptime = 1 / density" to convert a table or real-time
            control source from density to hop time.

    * p13 = grain input time jitter
            Maximum randomly determined amount to add or subtract from the
            input start time for a grain.

    * p14 = grain output time jitter
            Maximum randomly determined amount to add or subtract from the
            output start time for a grain, which is controlled by p12 (grain
            hop time).

    * p15 = grain duration minimum
    * p16 = grain duration maximum

    * p17 = grain amplitude multiplier minimum
    * p18 = grain amplitude multiplier maximum

    * p19 = grain transposition (in linear octaves, relative to 0)
            [optional; if missing, no transposition]

      p20 = grain transposition collection
            If this is a table, it contains a list of transpositions (in oct.pc)
            from which to select randomly.  If it's not a table, it's ignored.
            The table cannot be updated dynamically.  The value of p19
            (transposition) affects the collection.
            [optional]

    * p21 = grain transposition jitter
            Maximum randomly determined amount to add or subtract from the
            current transposition value.  If p20 (transposition collection)
            is active, then jitter controls how much of the collection to
            choose from.  Note that jitter is still in linear octaves, although
            the transposition collection is in oct.pc.  For example, if the
            collection is [0.00, 0.02, 0.05, 0.07], then a jitter value of
            octpch(0.05) will cause only the first three transpositions to be
            chosen, whereas a jitter value of octpch(0.07) would cause all four
            to be chosen.
            [optional; if missing, no transposition jitter]

      p22 = random seed (integer)
            [optional; if missing, uses zero for the seed]

    * p23 = grain pan minimum (pctleft: 0-1)
    * p24 = grain pan maximum
            [optional, ignored if mono output; if both missing, min = 0 and
            min = 1; if max missing, max = min]

      p25 = use 3rd-order interpolation, instead of 2nd-order, when transposing.
            This is more CPU-intensive, and often doesn't make a noticeable
            difference.  (0: use 2nd-order, 1: use 3rd-order)
            [optional; 2nd-order interp is the default]

   John Gibson <johgibso at indiana dot edu>, 1/29/05
*/
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <ugens.h>
#include <PField.h>
#include <rt.h>
#include <rtdefs.h>
#include "GRANULATE.h"
#include "grainstream.h"
//#define NDEBUG     // disable asserts
#include <assert.h>

//#define DEBUG

#define PRESERVE_GRAIN_DURATION  true     // regardless of transposition

#define USAGE_MESSAGE \
"Usage:\n" \
"  GRANULATE(start, inskip, dur, amp, sound_table, num_chans, input_chan,\n" \
"    window_start_time, window_end_time, wraparound, traversal_rate,\n" \
"    grain_env, grain_hoptime, grain_input_jitter, grain_output_jitter,\n" \
"    grain_dur_min, grain_dur_max, grain_amp_min, grain_amp_max,\n" \
"    grain_transposition, grain_transposition_collection,\n" \
"    grain_transposition_jitter, random_seed, grain_pan_min, grain_pan_max\n"


GRANULATE::GRANULATE()
	: _branch(0), _curwinstart(-DBL_MAX), _curwinend(-DBL_MAX), _stream(NULL),
   _block(NULL), _keepgoing(true), _stopped(false)
{
}


GRANULATE::~GRANULATE()
{
   delete _stream;
   delete [] _block;
}


int GRANULATE::init(double p[], int n_args)
{
   _nargs = n_args;
   if (_nargs < 19)
      return die("GRANULATE", USAGE_MESSAGE);
   const double outskip = p[0];
   const double dur = p[2];
   const int numinchans = int(p[5]);
   const int seed = _nargs > 22 ? int(p[22]) : 0;
   const bool use3rdOrderInterp = _nargs > 25 ? bool(p[25]) : false;

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;

   if (outputChannels() > 2)
      return die("GRANULATE", "You can have only mono or stereo output.");
   _stereoOut = (outputChannels() == 2);

   int length;
   double *table = (double *) getPFieldTable(4, &length);
   if (table == NULL)
      return die("GRANULATE", "You must create a table containing the sound "
                              "to granulate.");
   _stream = new GrainStream(SR, table, length, numinchans, outputChannels(),
                             PRESERVE_GRAIN_DURATION, seed, use3rdOrderInterp);

   table = (double *) getPFieldTable(11, &length);
   if (table == NULL)
      return die("GRANULATE", "You must create a table containing the grain "
                              "envelope.");
   _stream->setGrainEnvelopeTable(table, length);

   if (_nargs > 20) {
      table = (double *) getPFieldTable(20, &length);
      if (table != NULL)
         _stream->setGrainTranspositionCollection(table, length);
   }

   return nSamps();
}


void GRANULATE::doupdate()
{
   double p[_nargs];
   update(p, _nargs, kInskip | kAmp | kInChan | kWinStart | kWinEnd | kWrap
         | kTraversal | kHopTime | kInJitter | kOutJitter | kMinDur | kMaxDur
         | kMinAmp | kMaxAmp | kTransp | kTranspJitter | kMinPan | kMaxPan);

   _amp = p[3];
   _stream->setInputChan(int(p[6]));
   if (p[7] != _curwinstart || p[8] != _curwinend) {
      _stream->setWindow(p[7], p[8]);
      _curwinstart = p[7];
      _curwinend = p[8];
   }
   _stream->setInskip(p[1]);     // do this after setting window
   _stream->setWraparound(bool(p[9]));
   _stream->setTraversalRateAndGrainHop(p[10], p[12]);
   _stream->setInputJitter(p[13]);
   _stream->setOutputJitter(p[14]);
   _stream->setGrainDuration(p[15], p[16]);
   _stream->setGrainAmp(p[17], p[18]);
   if (_nargs > 19)
      _stream->setGrainTransposition(p[19]);
   if (_nargs > 21)
      _stream->setGrainTranspositionJitter(p[21]);
   if (_nargs > 23 && _stereoOut) {
      const double min = p[23];
      const double max = _nargs > 24 ? p[24] : min;
      _stream->setGrainPan(min, max);
   }
}


int GRANULATE::configure()
{
   _block = new float [RTBUFSAMPS * outputChannels()];
   return _block ? 0 : -1;
}


inline const int min(const int a, const int b)
{
   return a < b ? a : b;
}


#if 0    // shows how to do single-frame I/O, but we use block I/O instead.

int GRANULATE::run()
{
   const int frames = framesToRun();
   const int outchans = outputChannels();
   int i;
   for (i = 0; i < frames; i++) {
      if (--_branch <= 0) {
         doupdate();
         _branch = getSkip();
      }

      // If we're not in wrap mode, this returns false when it's time to stop.
      _keepgoing = _stream->prepare();

      float out[outchans];
      if (outchans == 2) {
         out[0] = _stream->lastL() * _amp;
         out[1] = _stream->lastR() * _amp;
      }
      else
         out[0] = _stream->lastL() * _amp;

      rtaddout(out);
      increment();

      if (!_keepgoing) {
// FIXME: how do we remove note from RTcmix queue?
         break;
      }
   }

   return i;
}

#else

int GRANULATE::run()
{
   // NOTE: Without a lot more code, we can't guarantee that doupdate will
   // be called exactly every getSkip() samples, the way we can when not doing
   // block I/O.  But this seems worth sacrificing for the clear performance
   // improvement that block I/O offers.

   const int frames = framesToRun();
   int blockframes = min(frames, getSkip());
   int framesdone = 0;
   while (1) {
      if (_branch <= 0) {
         doupdate();
         _branch = getSkip();
      }
      _branch -= blockframes;

      // If we're not in wrap mode, this returns false when it's time to stop.
      if (_keepgoing)
         _keepgoing = _stream->processBlock(_block, blockframes, _amp);
      rtbaddout(_block, blockframes);
      increment(blockframes);
      if (!_keepgoing && !_stopped) {
// FIXME: how do we remove note from RTcmix queue?
         const int samps = RTBUFSAMPS * outputChannels();
         for (int i = 0; i < samps; i++)
            _block[i] = 0.0f;
         rtcmix_advise("GRANULATE", "Reached end in non-wrap mode; stopping output.");
         _stopped = true;
      }
      framesdone += blockframes;
      if (framesdone == frames)
         break;
      assert(framesdone < frames);
      const int remaining = frames - framesdone;
      if (remaining < blockframes)
         blockframes = remaining;
   }
   return frames;
}

#endif

Instrument *makeGRANULATE()
{
   GRANULATE *inst;

   inst = new GRANULATE();
   inst->set_bus_config("GRANULATE");

   return inst;
}

void rtprofile()
{
   RT_INTRO("GRANULATE", makeGRANULATE);
}

