/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/* rev for v2.3 by JGG */

#include <globals.h>
#include <prototypes.h>
#include <ugens.h>
#include <stdio.h>
#include <assert.h>
#include "../rtstuff/Instrument.h"
#include "../rtstuff/rtdefs.h"


#define INCHANS_DISCREPANCY_WARNING "\
The bus config for this instrument specifies %d input channels, \n\
but its input source has %d channels. Setting input channels to %d..."



/* ----------------------------------------------------------- rtsetinput --- */
/* Instruments call this to set their input file pointer (like setnote in cmix).
   Returns 0 if ok, -1 if error.
*/
int
Instrument::rtsetinput(float start_time, Instrument *inst)
{
   int   auxin_count = inst->GetBusSlot()->auxin_count;
   int   in_count = inst->GetBusSlot()->in_count;
   char  *inst_name = NULL;      // FIXME: need this for better msgs

   if (auxin_count == 0 && in_count == 0)
      die(inst_name, "This instrument requires input from either an in bus "
                      "or an aux bus.\nChange this with bus_config().");

   if (auxin_count > 0) {
      if (start_time != 0.0)
         die(inst_name, "Input start must be 0 when reading from an aux bus.");
   }

   if (in_count > 0) {
      int src_chans;
      int index = get_last_input_index();

#ifdef LINUX
      if (index < 0 || inputFileTable[index].fd < 1)
         die(inst_name, "No input source open for this instrument!");
#endif
#ifdef SGI
      if (index < 0)
         die(inst_name, "No input source open for this instrument!");
      if ((inputFileTable[index].fd < 1)
                         && (inputFileTable[index].fd != AUDIO_DEVICE_FD))
         die(inst_name, "No input source open for this instrument!");
#endif

      /* File or audio device was opened in rtinput(). Here we store the
         index into the inputFileTable for the file or device.
      */
      inst->fdIndex = index;

      /* Fill in relevant data members of instrument class. */
      inst->inputsr = inputFileTable[index].srate;

      src_chans = inputFileTable[index].chans;

      if (inputFileTable[index].is_audio_dev) {
         if (start_time != 0.0)
            die(inst_name, "Input start must be 0 when reading from the "
                           "real-time audio device.");
      }
      else {
         int datum_size, inskip_frames;

         if (inst->inputchans != src_chans) {
#ifdef NOMORE // pointless ifdef IGNORE_BUS_COUNT_FOR_FILE_INPUT in rtgetin.C
            advise(inst_name, INCHANS_DISCREPANCY_WARNING, inst->inputchans,
                                                        src_chans, src_chans);
#endif
            inst->inputchans = src_chans;
         }

         inst->sfile_on = 1;

         inskip_frames = (int) (start_time * inst->inputsr);

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
                            + (inskip_frames * inst->inputchans * datum_size);

         if (start_time >= inputFileTable[index].dur)
            warn(inst_name, "Attempt to read past end of input file: %s",
                                             inputFileTable[index].filename);
      }

      /* Increment the reference count for this file. */
      inputFileTable[index].refcount++;
   }

   return 0;
}

