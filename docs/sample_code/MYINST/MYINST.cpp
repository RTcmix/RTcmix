/* MYINST - sample code for a very basic instrument

   All it does is copy samples from one file (or audio device) to
   another, processing only 1 input channel for a given note. You
   can choose the input channel and a pctleft value for the output.
   Shows how to implement setline (and any other makegen control).
   Please send me suggestions for comment clarification.

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = input channel [optional, default is 0]
   p5 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   JGG <johngibson@virginia.edu>, 12 April 2000
*/
#include <iostream.h>        /* needed only for cout, etc. if you want it */
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <math.h>
#include <mixerr.h>
#include <Instrument.h>      /* the base class for this instrument */
#include "MYINST.h"          /* declarations for this instrument class */
#include <rt.h>
#include <rtdefs.h>


/* Construct an instance of this instrument and initialize some variables. */
MYINST :: MYINST() : Instrument()
{
   in = NULL;
   branch = 0;
}


/* Destruct an instance of this instrument, freeing memory for the
   input buffer.
*/
MYINST :: ~MYINST()
{
   delete [] in;
}


/* Called by the scheduler to initialize the instrument. Things done here:
     - read, store and check pfields
     - set input and output file (or bus) pointers
     - init makegen tables and other instrument-specific things
     - set control rate counter
   If there's an error here (like invalid pfields), call die() to report
   the error and exit. If you just want to warn the user and keep going,
   call warn() or rterror() with a message.
*/
int MYINST :: init(float p[], int n_args)
{
   float outskip, inskip, dur;

   /* Store pfields in variables, to allow for easy pfield renumbering.
      You should retain the RTcmix numbering convention for the first
      4 pfields: outskip, inskip, dur, amp; or, for instruments that 
      take no input: outskip, dur, amp.
   */
   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];

   /* Here's how to handle optional pfields: */
   inchan = n_args > 4 ? (int) p[4] : 0;             /* default is chan 0 */
   pctleft = n_args > 5 ? p[5] : 0.5;                /* default is .5 */

   /* Tell scheduler when to start this inst. <nsamps> is number of sample
      frames that will be written to output (dur * SR).
   */
   nsamps = rtsetoutput(outskip, dur, this);

   /* Set file pointer on audio input. */
   rtsetinput(inskip, this);

   /* Make sure requested input channel number is valid for this input file.
      inputChannels() gives the total number of input channels, initialized
      in rtsetinput.  The die function reports the error and exits the
      program.
   */
   if (inchan >= inputChannels())
      die("MYINST", "You asked for channel %d of a %d-channel file.",
                                                      inchan, inputChannels());

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
      advise("MYINST", "Setting phrase curve to all 1's.");

   /* Set control rate counter. */
   skip = (int) (SR / (float) resetval);

   aamp = amp;                  /* in case amparray == NULL */

   return nsamps;
}

int MYINST :: configure()
{
	/* You MUST call the base class's configure method here. */
	Instrument :: configure();
	/*
      Allocate the input buffer. We do this here, instead of in the
      ctor or init method, to reduce the memory demands of the inst.
	*/
	if (in == NULL)
		in = new float [RTBUFSAMPS * inputChannels()];

	return 1;	/* we MUST return 1 on success, and 0 on failure */
}

/* Called by the scheduler for every time slice in which this instrument
   should run. This is where the real work of the instrument is done.
*/
int MYINST :: run()
{
   int   i, samps;
   float insig;
   float out[2];        /* Space for only 2 output chans! */

   /* You MUST call the base class's run method here. */
   Instrument::run();

   /* FramesToRun() gives the number of sample frames -- 1 sample for each
      channel -- that we have to write during this scheduler time slice.
   */
   samps = framesToRun() * inputChannels();

   /* Read <samps> samples from the input file (or audio input device). */
   rtgetin(in, this, samps);

   /* Each loop iteration processes 1 sample frame. */
   for (i = 0; i < samps; i += inputChannels()) {

      /* Every <skip> frames, update the amplitude envelope, if there
         is one. This is also the place to update other values from
         makegen curves, such as filter sweep, glissando, etc, and
         to do any rtupdate pfield updating. (See insts.base/WAVETABLE
         for an example of this.)
      */
      if (--branch < 0) {
         if (amparray)
            aamp = tablei(currentFrame(), amparray, amptabs) * amp;
         branch = skip;
      }

      /* Grab the current input sample, scaled by the amplitude multiplier. */
      insig = in[i + inchan] * aamp;

      /* Just copy it to the output array with no processing. */
      out[0] = insig;

      /* If we have stereo output, use the pctleft pfield to pan.
         (Note: insts.jg/PAN/PAN.C shows a better method of panning,
         using constant power panning controlled by a makegen.)
      */
      if (outputChannels() == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }

      /* Write this sample frame to the output buffer. */
      rtaddout(out);

      /* Increment the count of sample frames this instrument has written. */
      increment();
   }

   return framesToRun();
}


/* The scheduler calls this to create an instance of this instrument,
   and to set up the bus-routing fields in the base Instrument class.
   This happens for every "note" in a score.
*/
Instrument *makeMYINST()
{
   MYINST *inst;

   inst = new MYINST();
   inst->set_bus_config("MYINST");

   return inst;
}


/* The rtprofile introduces this instrument to the RTcmix core, and
   associates a Minc name (in quotes below) with the instrument. This
   is the name the instrument goes by in a Minc script.
*/
void rtprofile()
{
   RT_INTRO("MYINST", makeMYINST);
}


