/* GRANSYNTH - simple granular synthesis instrument

   Any parameter marked with '*' can receive updates from a real-time
   control source.

      p0  = output start time
      p1  = total duration
    * p2  = amplitude
      p3  = oscillator waveform (e.g., maketable("wave", ...))

      The following parameters determine the character of individual grains.

      p4  = grain envelope table

    * p5  = grain hop time (time between successive grains).  This is the 
            inverse of grain density (grains per second); you can use
            "hoptime = 1 / density" to convert a table or real-time
            control source from density to hop time.

    * p6  = grain output time jitter
            Maximum randomly determined amount to add or subtract from the
            output start time for a grain, which is controlled by p5 (grain
            hop time).

    * p7  = grain duration minimum
    * p8  = grain duration maximum

    * p9  = grain amplitude multiplier minimum
    * p10 = grain amplitude multiplier maximum

    * p11 = grain pitch (linear octaves)

      p12 = grain transposition collection
            If this is a table, it contains a list of transpositions (in oct.pc)
            from which to select randomly.  If it's not a table, it's ignored.
            The table cannot be updated dynamically.  The values from the 
            table are used to transpose p11 (pitch).
            [optional]

    * p13 = grain pitch jitter
            Maximum randomly determined amount to add or subtract from the
            current pitch (in linear octaves).  If p12 (transposition
            collection) is active, then jitter controls how much of the
            collection to choose from.  Note that jitter is still in linear
            octaves, although the transposition collection is in oct.pc.
            For example, if the collection is [0.00, 0.02, 0.05, 0.07], then a
            jitter value of octpch(0.05) will cause only the first three
            transpositions to be chosen, whereas a jitter value of octpch(0.07)
            would cause all four to be chosen.
            [optional; if missing, no pitch jitter]

      p14 = random seed (integer)
            [optional; if missing, uses system clock]

    * p15 = grain pan minimum (pctleft: 0-1)
    * p16 = grain pan maximum
            [optional, ignored if mono output; if both missing, min = 0 and
            min = 1; if max missing, max = min]

   John Gibson <johgibso at indiana dot edu>, 2/8/05
*/
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <ugens.h>
#include <PField.h>
#include <rt.h>
#include <rtdefs.h>
#include "GRANSYNTH.h"
#include "synthgrainstream.h"
//#define NDEBUG     // disable asserts
#include <assert.h>

#define DEBUG


#define USAGE_MESSAGE \
"Usage:\n" \
"  GRANSYNTH(start, dur, amp, waveform, grain_env, grain_hoptime,\n" \
"    grain_output_jitter, grain_dur_min, grain_dur_max, grain_amp_min,\n" \
"    grain_amp_max, grain_pitch, grain_transposition_collection,\n" \
"    grain_pitch_jitter, random_seed, grain_pan_min, grain_pan_max\n"


GRANSYNTH::GRANSYNTH()
	: _branch(0), _stream(NULL), _block(NULL)
{
}


GRANSYNTH::~GRANSYNTH()
{
   delete _stream;
   delete [] _block;
}


int GRANSYNTH::init(double p[], int n_args)
{
   _nargs = n_args;
   if (_nargs < 11 || _nargs > 17)
      return die("GRANSYNTH", USAGE_MESSAGE);
   const double outskip = p[0];
   const double dur = p[1];
   const int seed = _nargs > 14 ? int(p[14]) : 0;

   if (rtsetoutput(outskip, dur, this) == -1)
      return DONT_SCHEDULE;

   if (outputChannels() > 2)
      return die("GRANSYNTH", "You can have only mono or stereo output.");
   _stereoOut = (outputChannels() == 2);

   int length;
   double *table = (double *) getPFieldTable(3, &length);
   if (table == NULL)
      return die("GRANSYNTH", "You must create a table containing the "
                              "oscillator waveform.");
   _stream = new SynthGrainStream(SR, table, length, outputChannels(), seed);

   table = (double *) getPFieldTable(4, &length);
   if (table == NULL)
      return die("GRANSYNTH", "You must create a table containing the grain "
                              "envelope.");
   _stream->setGrainEnvelopeTable(table, length);

   if (_nargs > 12) {
      table = (double *) getPFieldTable(12, &length);
      if (table != NULL)
         _stream->setGrainTranspositionCollection(table, length);
   }

   return nSamps();
}


void GRANSYNTH::doupdate()
{
   double p[_nargs];
   update(p, _nargs, kAmp | kHopTime | kOutJitter | kMinDur | kMaxDur
         | kMinAmp | kMaxAmp | kPitch | kPitchJitter | kMinPan | kMaxPan);

   _amp = p[2];
   _stream->setGrainHop(p[5]);
   _stream->setOutputJitter(p[6]);
   _stream->setGrainDuration(p[7], p[8]);
   _stream->setGrainAmp(p[9], p[10]);
   _stream->setGrainPitch(p[11]);
   if (_nargs > 13)
      _stream->setGrainPitchJitter(p[13]);
   if (_nargs > 15 && _stereoOut) {
      const double min = p[15];
      const double max = _nargs > 16 ? p[16] : min;
      _stream->setGrainPan(min, max);
   }
}


int GRANSYNTH::configure()
{
   _block = new float [RTBUFSAMPS * outputChannels()];
   return _block ? 0 : -1;
}


inline const int min(const int a, const int b)
{
   return a < b ? a : b;
}


#if 0    // shows how to do single-frame I/O, but we use block I/O instead.

int GRANSYNTH::run()
{
   const int frames = framesToRun();
   const int outchans = outputChannels();
   int i;
   for (i = 0; i < frames; i++) {
      if (--_branch <= 0) {
         doupdate();
         _branch = getSkip();
      }

      _stream->prepare();

      float out[outchans];
      if (outchans == 2) {
         out[0] = _stream->lastL() * _amp;
         out[1] = _stream->lastR() * _amp;
      }
      else
         out[0] = _stream->lastL() * _amp;

      rtaddout(out);
      increment();
   }

   return i;
}

#else

int GRANSYNTH::run()
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

      _stream->processBlock(_block, blockframes, _amp);
      rtbaddout(_block, blockframes);
      increment(blockframes);
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

Instrument *makeGRANSYNTH()
{
   GRANSYNTH *inst;

   inst = new GRANSYNTH();
   inst->set_bus_config("GRANSYNTH");

   return inst;
}

#ifndef MAXMSP
void rtprofile()
{
   RT_INTRO("GRANSYNTH", makeGRANSYNTH);
}
#endif
