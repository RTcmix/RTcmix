/* MYSYNTH - sample code for a very basic synthesis instrument

   All it does is write noise into the output buffer, with optional
   static panning. Shows how to implement setline (and any other makegen
   control). Please send me suggestions for comment clarification.

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   JGG <johngibson@virginia.edu>, 17 May 2000
*/
#include <iostream.h>        /* needed only for cout, etc. if you want it */
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MYSYNTH.h"         /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>


/* Construct an instance of this instrument and initialize a variable. */
MYSYNTH :: MYSYNTH() : Instrument()
{
   branch = 0;
}


/* Destruct an instance of this instrument. Here's where to free any memory
   you may have allocated.
*/
MYSYNTH :: ~MYSYNTH()
{
}


/* Called by the scheduler to initialize the instrument. Things done here:
     - read, store and check pfields
     - set output file (or bus) pointer
     - init makegen tables and other instrument-specific things
     - set control rate counter
   If there's an error here (like invalid pfields), call die() to report
   the error and return. If you just want to warn the user and keep going,
   call warn() with a message.
*/
int MYSYNTH :: init(double p[], int n_args)
{
   float outskip, dur;

   /* Store pfields in variables, to allow for easy pfield renumbering.
      You should retain the RTcmix numbering convention for the first
      4 pfields: outskip, inskip, dur, amp; or, for instruments that 
      take no input: outskip, dur, amp.
   */
   outskip = p[0];
   dur = p[1];
   amp = p[2];

   /* Here's how to handle an optional pfield: */
   pctleft = n_args > 3 ? p[3] : 0.5;                /* default is .5 */

   /* Tell scheduler when to start this inst. <nsamps> is number of sample
      frames that will be written to output (dur * SR).
   */
   nsamps = rtsetoutput(outskip, dur, this);

   /* Set up to use the array of amplitude multipliers created by setline
      (which is just an alias to gen18). If function table hasn't been
      created (e.g., there's no setline call), we'll pretend there's an
      array containing all 1's.

      floc(1) returns a pointer to the array created when the score contains
      "makegen(1, ...)" (or "setline(...)").
      fsize(1) is the number of elements in this array.
      tableset() initializes the apparatus used for indexing the array
      based on the current sample, used in the run method loop.
   */
   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(dur, lenamp, amptabs);
   }
   else
      advise("MYSYNTH", "Setting phrase curve to all 1's.");

   /* Set control rate counter. */
   skip = (int) (SR / (float) resetval);

   aamp = amp;                  /* in case amparray == NULL */

   return nsamps;
}


/* Called by the scheduler for every time slice in which this instrument
   should run. This is where the real work of the instrument is done.
*/
int MYSYNTH :: run()
{
   int   i;
   float out[2];        /* Space for only 2 output chans! */

   /* You no longer call the base class's run method here! */

   /* FramesToRun() gives the number of sample frames -- 1 sample for each
      channel -- that we have to write during this scheduler time slice.
      Each loop iteration outputs 1 sample frame.
   */
   for (i = 0; i < FramesToRun(); i++) {

      /* Every <skip> frames, update the amplitude envelope, if there
         is one. This is also the place to update other values from
         makegen curves, such as filter sweep, glissando, etc, and
         to do any rtupdate pfield updating. (See insts.base/WAVETABLE
         for an example of this.)
      */
      if (--branch < 0) {
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;
         branch = skip;
      }

      /* Write a random number, scaled by the amplitude multiplier, into
         the output array.
      */
      out[0] = rrand() * aamp;

      /* If we have stereo output, use the pctleft pfield to pan.
         (Note: insts.jg/PAN/PAN.C shows a better method of panning,
         using constant power panning controlled by a makegen.)
      */
      if (OutputChannels() == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      /* Write this sample frame to the output buffer. */
      rtaddout(out);

      /* Increment the count of sample frames this instrument has written. */
      increment();
   }

   return FramesToRun();
}


/* The scheduler calls this to create an instance of this instrument,
   and to set up the bus-routing fields in the base Instrument class.
   This happens for every "note" in a score.
*/
Instrument *makeMYSYNTH()
{
   MYSYNTH *inst;

   inst = new MYSYNTH();
   inst->set_bus_config("MYSYNTH");

   return inst;
}


/* The rtprofile introduces this instrument to the RTcmix core, and
   associates a Minc name (in quotes below) with the instrument. This
   is the name the instrument goes by in a Minc script.
*/
void rtprofile()
{
   RT_INTRO("MYSYNTH", makeMYSYNTH);
}


