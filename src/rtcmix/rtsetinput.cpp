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

#define INCHANS_DISCREPANCY_WARNING "\
WARNING: The bus config for this instrument specifies %d input channels, \n\
but its input source has %d channels.\n"



/* ----------------------------------------------------------- rtsetinput --- */
/* Instruments call this to set their input file pointer (like setnote in cmix).
   Returns 0 if ok, -1 if error.
*/
int
rtsetinput(float start_time, Instrument *inst)
{
   int auxin_count = inst->bus_config->auxin_count;
   int in_count = inst->bus_config->in_count;

   if (auxin_count == 0 && in_count == 0) {
      fprintf(stderr, "This instrument requires input from either an in bus"
                      "or an aux bus.\n Change this with bus_config().\n");
      exit(1);
   }

   if (auxin_count > 0) {
      if (start_time != 0.0) {
         fprintf(stderr, "inskip must be 0 when reading from an aux bus.\n");
         exit(1);
      }
   }

   if (in_count > 0) {
      int src_chans;
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

      src_chans = inputFileTable[index].chans;

      if (inst->inputchans != src_chans) {
         fprintf(stderr, INCHANS_DISCREPANCY_WARNING, inst->inputchans,
                                                                  src_chans);
      }

      if (inputFileTable[index].is_audio_dev) {
         if (start_time != 0.0) {
            fprintf(stderr, "inskip must be 0 when reading from the real-time "
                            "audio device.\n");
            exit(1);
         }
      }
      else {
         int datum_size, inskip;

         inst->sfile_on = 1;

         inskip = (int) (start_time * inst->inputsr);

         /* sndlib always uses 2 for datum size, even if the actual size is
            different. However, we don't use sndlib to read float files, so
            their datum size is the real thing.
         */ 
         if (inputFileTable[index].is_float_format)
            datum_size = sizeof(float);
         else
            datum_size = 2;

         /* Offset is measured from the header size determined in rtinput(). */
         inst->fileOffset = inputFileTable[index].data_location
                            + (inskip * inst->inputchans * datum_size);

         if (start_time >= inputFileTable[index].dur)
            fprintf(stderr, "WARNING: Attempt to read past end of input "
                            "file: %s\n", inputFileTable[index].filename);
      }

      /* Increment the reference count for this file. */
      inputFileTable[index].refcount++;
   }

   return 0;
}

