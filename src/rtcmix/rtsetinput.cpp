/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* rev for v2.3 by JGG */

#include <globals.h>
#include <stdio.h>
#include <assert.h>
#include "../rtstuff/Instrument.h"
#include "../rtstuff/rtdefs.h"

extern "C" {
extern int get_last_input_index(void);       /* defined in rtinput.c */
}


/* ----------------------------------------------------------- rtsetinput --- */
/* Instruments call this to set their input file pointer (like setnote in cmix).
   Returns 0 if ok, -1 if error.
*/
int
rtsetinput(float start_time, Instrument *inst)
{
   if (inst->bus_config->auxin_count > 0) {
// FIXME: not sure we have to do anything here  -JGG
   }

   if (inst->bus_config->in_count > 0) {
      int index = get_last_input_index();

      if (index < 0 || inputFileTable[index].fd < 1) {
         fprintf(stderr, "No input source open for this instrument!\n");
         return -1;
      }

      /* File or audio device was opened in rtinput(). Here we store the
         index into the inputFileTable for the file or device.
      */
      inst->fdIndex = index;

      /* Fill in relevant data members of instrument class. */
      inst->inputsr = inputFileTable[index].srate;

// FIXME: not sure what to do here. inputchans is set according to what
// bus_config specifies. Currently, we just warn about the descrepancy,
// but maybe we should do something about it?  -JGG
      if (inst->inputchans != inputFileTable[index].chans) {
         fprintf(stderr, "WARNING: This instrument's bus config doesn't "
                         "match the number of input channels.\n");
      }

      if (!inputFileTable[index].is_audio_dev) {
         int datum_size = sizeof(short);
         int inskip = (int) (start_time * inst->inputsr);

         inst->sfile_on = 1;

         /* Offset is measured from location that is set up in rtinput(). */
         inst->fileOffset = inputFileTable[index].data_location
                            + (inskip * inst->inputchans * datum_size);

         if (start_time >= inputFileTable[index].dur)
            fprintf(stderr, "\nWARNING: Attempt to read past end of input "
                            "file: %s\n\n", inputFileTable[index].filename);
      }

      /* We also increment the reference count for this file. */
      inputFileTable[index].refcount++;
   }

   return 0;
}

