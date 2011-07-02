/* Test for the SoundIn object in objlib.  -JGG, 8/2/01 */
#include <stdio.h>
#include <stdlib.h>
#include <ugens.h>
#include <mixerr.h>
#include <Instrument.h>
#include "soundin.h"
#include <rt.h>
#include <rtdefs.h>

extern "C" {
   #include "getfilename.h"
}


soundin :: soundin() : Instrument()
{
   infile = NULL;
}


soundin :: ~soundin()
{
   delete infile;
}


int soundin :: init(float p[], int n_args)
{
   float outskip, inskip, dur;
   char  *infilename;

   outskip = p[0];
   inskip = p[1];
   dur = p[2];
   amp = p[3];
   inchan = (int) p[4];
   pctleft = p[5];

   nsamps = rtsetoutput(outskip, dur, this);

   infilename = get_filename();
   if (infilename == NULL)
      die("soundin", "Use \"openfile\" to specify an input file.");

   infile = new SoundIn(infilename, inskip);
   int numchans = infile->getChannels();
   if (inchan >= numchans)
      die("soundin", "You asked for channel %d of a %d-channel file.",
                                                      inchan, numchans);

   amparray = floc(1);
   if (amparray) {
      int lenamp = fsize(1);
      tableset(dur, lenamp, amptabs);
   }
   else
      advise("soundin", "Setting phrase curve to all 1's.");

   skip = (int)(SR / (float)resetval);

   return nsamps;
}


int soundin :: run()
{
   int   i, branch, rsamps;
   float aamp;
   float *sigs;
   float out[MAXBUS];

   Instrument :: run();

   aamp = amp;                  /* in case amparray == NULL */

   branch = 0;
   for (i = 0; i < framesToRun(); i++) {
      if (--branch < 0) {
         if (amparray)
            aamp = tablei(cursamp, amparray, amptabs) * amp;
         branch = skip;
      }
      sigs = infile->tick();  /* returns one frame's worth */
      out[0] = sigs[inchan];
      out[0] *= aamp;
      if (outputchans == 2) {
         out[1] = out[0] * (1.0 - pctleft);
         out[0] *= pctleft;
      }
      rtaddout(out);
      cursamp++;
   }

   return i;
}


Instrument *makesoundin()
{
   soundin *inst;

   inst = new soundin();
   inst->set_bus_config("soundin");

   return inst;
}


void
rtprofile()
{
   RT_INTRO("soundin", makesoundin);
}


