/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream.h>
#include "Instrument.h"
#include "rt.h"
#include "rtdefs.h"
#include <notetags.h>
#include <sndlibsupport.h>
#include <bus.h>
#include <assert.h>



Instrument :: Instrument()
{
   start = 0.0;
   dur = 0.0;
   cursamp = 0;
   chunksamps = 0;
   endsamp = 0;
   nsamps = 0;
   chunkstart = 0;
   output_offset = 0;

   sfile_on = 0;                // default is no input soundfile
   fdIndex = NO_DEVICE_FDINDEX;
   fileOffset = 0;

   inputsr = 0.0;
   inputchans = 0;
   outputchans = 0;

// FIXME: not clear we need inbuf - better to leave it on run method's stack?
   inbuf = NULL;
   outbuf = NULL;

   bus_config = NULL;

   if (tags_on) {
      pthread_mutex_lock(&pfieldLock);

      for (int i = 0; i < MAXPUPS; i++)  // initialize this element
         pupdatevals[curtag][i] = NOPUPDATE;
      mytag = curtag++;

      if (curtag >= MAXPUPARR)
         curtag = 1;            // wrap it around
      // 0 is reserved for all-note rtupdates

      pthread_mutex_unlock(&pfieldLock);
   }
}


Instrument :: ~Instrument()
{
   if (sfile_on)
      gone();                   // decrement input soundfile reference

   delete [] inbuf;
   delete [] outbuf;

// FIXME: Also...
// Call something that decrements refcount for bus_config, and if that
// reaches zero, and is no longer the most recent for that instname,
// then delete that bus_config node.
}


// Set the bus_config pointer to the right bus_config for this inst.
// Then set the inputchans and outputchans members accordingly.
//
// Instruments *must* call this from within their makeINSTNAME method. E.g.,
//
//     WAVETABLE *inst = new WAVETABLE();
//     inst->set_bus_config("WAVETABLE");
//
void Instrument :: set_bus_config(const char *inst_name)
{
   bus_config = get_bus_config(inst_name);

   inputchans = bus_config->in_count + bus_config->auxin_count;
   outputchans = bus_config->out_count + bus_config->auxout_count;
}


int Instrument :: init(float p[], short n_args)
{
   cout << "You haven't defined an init member of your Instrument class!"
                                                                   << endl;
   return -1;
}


// Instruments *must* call this at the beginning of their run methods,
// like this:
//
//    Instrument::run();
//
// This method allocates the instrument's private interleaved output buffer
// and inits a buffer status array.
// Note: We allocate here, rather than in ctor or init method, because this
// will mean less memory overhead before the inst begins playing.
//
int Instrument :: run()
{
   if (outbuf == NULL)
      outbuf = new BUFTYPE [RTBUFSAMPS * outputchans];

   obufptr = outbuf;

#ifdef NOTYET
   for (int i = 0; i < outputchans; i++)
      bufstatus[i] = 0;
#endif

   return 0;
}


void Instrument :: exec()
{
   run();
}


// Replacement for the old rtaddout (in rtaddout.C, now removed).
// This one copies (not adds) into the inst's outbuf. Later the
// schedular calls the insts addout method to add outbuf into the 
// appropriate output buses. Inst's *must* call the class run method
// before doing their own run stuff. (This is true even if they don't
// use rtaddout.)
// Assumes that <samps> contains exactly outputchans interleaved samples.
// Returns outputchans (i.e., number of samples written).
//
int Instrument :: rtaddout(BUFTYPE samps[])
{
   for (int i = 0; i < outputchans; i++)
      *obufptr++ = samps[i];
}


// Add signal from one channel of instrument's private interleaved buffer
// into the specified output bus.
//
void Instrument :: addout(BusType bus_type, int bus)
{
   int      samp, endframe, src_chan, buses;
   short    *bus_list;
   BufPtr   src, dest;

   assert(bus >= 0 && bus < MAXBUS);

   if (bus_type == BUS_AUX_OUT) {
      dest = aux_buffer[bus];
      buses = bus_config->auxout_count;
      bus_list = bus_config->auxout;
   }
   else {       // BUS_OUT
      dest = out_buffer[bus];
      buses = bus_config->out_count;
      bus_list = bus_config->out;
   }

   src_chan = -1;
   for (int i = 0; i < buses; i++) {
      if (bus_list[i] == bus) {
         src_chan = i;
         break;
      }
   }

   assert(src_chan != -1);
   assert(dest != NULL);

   endframe = output_offset + chunksamps;
   samp = src_chan;

// FIXME: pthread_mutex_lock dest buffer

   for (int frame = output_offset; frame < endframe; frame++) {
      dest[frame] += outbuf[samp];
      samp += outputchans;
   }

// FIXME: pthread_mutex_unlock dest buffer
}


float Instrument :: getstart()
{
   return start;
}

float Instrument :: getdur()
{
   return dur;
}

int Instrument :: getendsamp()
{
   return endsamp;
}

void Instrument :: setendsamp(int end)
{
   endsamp = end;
}

void Instrument :: setchunk(int csamps)
{
   chunksamps = csamps;
}

void Instrument :: setchunkstart(int csamps)
{
   chunkstart = csamps;
}

void Instrument :: set_output_offset(int offset)
{
   output_offset = offset;
}


// If the reference count on the file referenced by the instrument
// reaches zero, close the input soundfile and set the state to 
// make sure this is obvious
//
void Instrument :: gone()
{
#ifdef DEBUG
   printf("Instrument::gone(this=0x%x): index %d refcount = %d\n",
          this, fdIndex, inputFileTable[fdIndex].refcount);
#endif

   // BGG -- added this to prevent file closings in interactive mode
   // we don't know if a file will be referenced again in the future
   if (!rtInteractive) {
      if (fdIndex >= 0 && --inputFileTable[fdIndex].refcount <= 0) {
         if (inputFileTable[fdIndex].fd > 0) {
#ifdef DEBUG
            printf("\tclosing fd %d\n", inputFileTable[fdIndex].fd);
#endif
            clm_close(inputFileTable[fdIndex].fd);
         }
         if (inputFileTable[fdIndex].filename);
            free(inputFileTable[fdIndex].filename);
         inputFileTable[fdIndex].filename = NULL;
         inputFileTable[fdIndex].fd = NO_FD;
         inputFileTable[fdIndex].header_type = unsupported_sound_file;
         inputFileTable[fdIndex].data_format = snd_unsupported;
         inputFileTable[fdIndex].data_location = 0;
         inputFileTable[fdIndex].srate = 0.0;
         inputFileTable[fdIndex].chans = 0;
         inputFileTable[fdIndex].dur = 0.0;
         fdIndex = -1;
      }
   }
}

