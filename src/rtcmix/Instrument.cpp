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
#include <rtupdate.h>
#include <ugens.h>

/* ----------------------------------------------------------- Instrument --- */
Instrument :: Instrument()
{
   int i;
   for(i = 0; i < MAXNUMPARAMS; ++i)
   {
	   pfpathcounter[i] = 0;
	   cumulative_size[i] = 0;
	   newpvalue[i] = 0;
	   oldsamp[i] = 0;
   }
   j = 0;
   k = 0;

   start = 0.0;
   dur = 0.0;
   cursamp = 0;
   chunksamps = 0;
   i_chunkstart = 0;
   endsamp = 0;
   nsamps = 0;
   output_offset = 0;

   sfile_on = 0;                // default is no input soundfile
   fdIndex = NO_DEVICE_FDINDEX;
   fileOffset = 0;

   inputsr = 0.0;
   inputchans = 0;
   outputchans = 0;

   outbuf = NULL;

   bus_config = NULL;

   for (int i = 0; i < MAXBUS; i++)
      bufstatus[i] = 0;
   needs_to_run = 1;

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


/* ---------------------------------------------------------- ~Instrument --- */
Instrument :: ~Instrument()
{
   if (sfile_on)
      gone();                   // decrement input soundfile reference

   delete [] outbuf;

// FIXME: Also...
// Call something that decrements refcount for bus_config, and if that
// reaches zero, and is no longer the most recent for that instname,
// then delete that bus_config node.
}


/* ------------------------------------------------------- set_bus_config --- */
/* Set the bus_config pointer to the right bus_config for this inst.
   Then set the inputchans and outputchans members accordingly.
  
   Instruments *must* call this from within their makeINSTNAME method. E.g.,
  
       WAVETABLE *inst = new WAVETABLE();
       inst->set_bus_config("WAVETABLE");
*/
void Instrument :: set_bus_config(const char *inst_name)
{
  pthread_mutex_lock(&bus_slot_lock);
  bus_config = get_bus_config(inst_name);
  
  inputchans = bus_config->in_count + bus_config->auxin_count;
  outputchans = bus_config->out_count + bus_config->auxout_count;
  pthread_mutex_unlock(&bus_slot_lock);
}


/* ----------------------------------------------------------------- init --- */
int Instrument :: init(float p[], int n_args)
{
//   cout << "You haven't defined an init member of your Instrument class!"
  //                                                                 << endl;
   int i;
   for(i = 0; i < n_args; ++i)
   {
	   newpvalue[i] = p[i];	
	   oldpvalue[i][0] = 0;
	   oldpvalue[i][1] = p[i];
   }
   slot = piloc(instnum);
   return 777;
}


/* ------------------------------------------------------------------ run --- */
/* Instruments *must* call this at the beginning of their run methods,
   like this:
  
      Instrument::run();
  
   This method allocates the instrument's private interleaved output buffer
   and inits a buffer status array.
   Note: We allocate here, rather than in ctor or init method, because this
   will mean less memory overhead before the inst begins playing.
*/
int Instrument :: run()
{
   if (outbuf == NULL)
      outbuf = new BUFTYPE [RTBUFSAMPS * outputchans];

   obufptr = outbuf;

   for (int i = 0; i < outputchans; i++)
      bufstatus[i] = 0;

   needs_to_run = 0;

   return 0;
}


/* ----------------------------------------------------------------- exec --- */
/* Called from inTraverse to do one or both of these tasks:

      1) run the instrument for the current timeslice, and/or

      2) call addout to transfer a channel from the private instrument
         output buffer to one of the global output buffers.

   exec(), run() and addout() work together to decide whether a given
   call to exec needs to run or addout. The first call to exec for a
   given timeslice does both; subsequent calls during the same timeslice
   only addout.
*/
void Instrument :: exec(BusType bus_type, int bus)
{
   int done;

   if (needs_to_run)
      run();

   addout(bus_type, bus);

   /* Decide whether we'll call run() next time. */
   done = 1;
   for (int i = 0; i < outputchans; i++) {
      if (bufstatus[i] == 0) {
         done = 0;
         break;
      }
   }
   if (done)
      needs_to_run = 1;
}


/* ------------------------------------------------------------- rtaddout --- */
/* Replacement for the old rtaddout (in rtaddout.C, now removed).
   This one copies (not adds) into the inst's outbuf. Later the
   scheduler calls the insts addout method to add outbuf into the 
   appropriate output buses. Inst's *must* call the class run method
   before doing their own run stuff. (This is true even if they don't
   use rtaddout.)
   Assumes that <samps> contains at least outputchans interleaved samples.
   Returns outputchans (i.e., number of samples written).
*/
int Instrument :: rtaddout(BUFTYPE samps[])
{
   for (int i = 0; i < outputchans; i++)
      *obufptr++ = samps[i];

   return outputchans;
}


/* --------------------------------------------------------------- addout --- */
/* Add signal from one channel of instrument's private interleaved buffer
   into the specified output bus.
*/
void Instrument :: addout(BusType bus_type, int bus)
{
   int      samp_index, endframe, src_chan, buses;
   short    *bus_list;
   BufPtr   src, dest;

   assert(bus >= 0 && bus < MAXBUS);

   if (bus_type == BUS_AUX_OUT) {
      dest = aux_buffer[bus];
      buses = bus_config->auxout_count;
      bus_list = bus_config->auxout;
   }
   else {       /* BUS_OUT */
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
   samp_index = src_chan;

// FIXME: pthread_mutex_lock dest buffer

   for (int frame = output_offset; frame < endframe; frame++) {
      dest[frame] += outbuf[samp_index];
      samp_index += outputchans;
   }

// FIXME: pthread_mutex_unlock dest buffer

   /* Show exec() that we've written this chan. */
   bufstatus[src_chan] = 1;
}


/* ------------------------------------------------------------- getstart --- */
float Instrument :: getstart()
{
   return start;
}

/* --------------------------------------------------------------- getdur --- */
float Instrument :: getdur()
{
   return dur;
}

/* ----------------------------------------------------------- getendsamp --- */
int Instrument :: getendsamp()
{
   return endsamp;
}

/* ----------------------------------------------------------- setendsamp --- */
void Instrument :: setendsamp(int end)
{
  pthread_mutex_lock(&endsamp_lock);
  endsamp = end;
  pthread_mutex_unlock(&endsamp_lock);
}

/* ------------------------------------------------------------- setchunk --- */
void Instrument :: setchunk(int csamps)
{
   chunksamps = csamps;
}

/* ------------------------------------------------------ set_ichunkstart --- */
void Instrument :: set_ichunkstart(int csamps)
{
   i_chunkstart = csamps;
}

/* ---------------------------------------------------- set_output_offset --- */
void Instrument :: set_output_offset(int offset)
{
   output_offset = offset;
}

/* ----------------------------------------------------------------- gone --- */
/* If the reference count on the file referenced by the instrument
   reaches zero, close the input soundfile and set the state to 
   make sure this is obvious.
*/
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
         inputFileTable[fdIndex].is_float_format = 0;
         inputFileTable[fdIndex].data_location = 0;
         inputFileTable[fdIndex].srate = 0.0;
         inputFileTable[fdIndex].chans = 0;
         inputFileTable[fdIndex].dur = 0.0;
         fdIndex = NO_DEVICE_FDINDEX;
      }
   }
}

/* -------------------------------------------------------------------- */

void Instrument::set_instnum(char* name)
{
	inst_list *tmp;
//	printf("firstname = %s\n", name);
//	printf("curinst = %i\n", curinst);
	if(ilist == NULL)
	{	
//		printf("one\n");
		ilist = (struct inst_list *) malloc(sizeof(struct inst_list));
		ilist->next = NULL;
		ilist->num = curinst++;
//		printf("curinst = %i\n", curinst);
	    ilist->name = name;
//		printf("inum = %i\n", ilist->num);
//		printf("iname = %s\n", ilist->name);
	}
	tmp = ilist;
//	printf("two\n");
//	printf("name = %s \n", name);
	while((strcmp(name, tmp->name) != 0) && (tmp->next != NULL))
	{
//		printf("two\n");
		tmp = tmp->next;
	}
//	printf("four\n");
	if(strcmp(name, tmp->name) == 0)
	{
		instnum = tmp->num;
//		printf("tnum = %i\n", instnum);
//		printf("tname = %s\n,", name);
	}
	else
	{	  
		tmp->next = (struct inst_list *) malloc(sizeof(struct inst_list));
		tmp = tmp->next;
		tmp->next = NULL;
		tmp->num = curinst++;
//		printf("curinst =%i\n", curinst);
		tmp->name = name;
		ilist->next = tmp;
		instnum = tmp->num;
//		printf("num = %i\n", instnum);
//		printf("name = %s\n", tmp->name);
	}
	return;
}


void Instrument::pi_path_update(int pval)
{
	int i;
//	printf("mytag = %i\n", mytag);
//	printf("instnum = %i\n", instnum);
//	printf("slot = %i\n", slot);
	if(slot < 0)
		return;
	
//	printf("piarray_size[%i][%i][%i] = %i\n\n\n", slot, pval, k
//                                            , piarray_size[slot][pval][k]);
	while((piarray_size[slot][pval][k] != 0) 
         && (parray_size[mytag][pval][k] == 0))
	{

//		printf("parraysize = %i\n", parray_size[mytag][pval]);
//		printf("piarraysize = %i\n", piarray_size[slot][pval][k]);
		
//		printf("piarray_size[%i][%i][%i] = %i\n", slot, pval, k
//                                            , piarray_size[slot][pval][k]);

		gen_type[mytag][pval][k] = igen_type[slot][pval][k];
		parray_size[mytag][pval][k] = piarray_size[slot][pval][k];
		numcalls[mytag][pval] = numinstcalls[slot][pval];
//		printf("k = %i\n", k);
		for(i = 0; i < cum_piarray_size[slot][pval]; ++i)
		{
			pfpath[mytag][pval][i][0] = pipath[slot][pval][i][0];
			pfpath[mytag][pval][i][1] = pipath[slot][pval][i][1];
			printf("pipath[%i][%i][%i][0] = %f\t", slot, pval, i
				, pipath[slot][pval][i][0]);
			printf("pipath[%i][%i][%i][1] = %f\n", slot, pval, i
				, pipath[slot][pval][i][1]);
			printf("pfpath[%i][%i][%i][0] = %f\t", mytag, pval, i
				, pfpath[mytag][pval][i][0]);
			printf("pfpath[%i][%i][%i][1] = %f\n", mytag, pval, i
				, pfpath[mytag][pval][i][1]);
		}
		k++;
	}

	


	return;
}

void Instrument::pf_path_update(int tag, int pval)
{
	float time, increment;
	float updates_per_second; 
	int i, incr_j;
	double table_val, diff, difftime;
	pi_path_update(pval);
	incr_j = 0;
//	printf("tag = %i\n", tag);
	
	if(gen_type[tag][pval][j] == 0 && gen_type[0][pval][j] == 0) 
                                            // linear interpolation of values
	{
		if(parray_size[tag][pval][j] > pfpathcounter[pval])
		{
			time = cursamp / SR + start;

			if(time < pfpath[tag][pval][0][0]) // before inst reaches first 
				return;						   // time specified in pfpath

			updates_per_second = resetval;
			increment = 1 / ((pfpath[tag][pval][cumulative_size[pval]][0] 
							 - oldpvalue[pval][0]) * updates_per_second);

			increment *= pfpath[tag][pval][cumulative_size[pval]][1] 
						 - oldpvalue[pval][1];

		
			newpvalue[pval] += increment;

			pupdatevals[tag][pval] = newpvalue[pval];

			if(time >= pfpath[tag][pval][cumulative_size[pval]][0])
			{
				pupdatevals[tag][pval] = 
                             pfpath[tag][pval][cumulative_size[pval]][1];

				oldpvalue[pval][0] = 
                             pfpath[tag][pval][cumulative_size[pval]][0];

				pfpathcounter[pval]++; 
				cumulative_size[pval]++;
				newpvalue[pval] = pupdatevals[tag][pval];
				oldpvalue[pval][1] = newpvalue[pval];
			
			}
		}

		if(parray_size[0][pval][j] > pfpathcounter[pval])
		{		
			time = cursamp / SR + start;

			if(time < pfpath[tag][pval][0][0]) // before inst reaches first 
				return;						   // time specified in pfpath

			updates_per_second = resetval;
			increment = 1 / ((pfpath[0][pval][cumulative_size[pval]][0] 
							 - oldpvalue[pval][0]) * updates_per_second);

			increment *= pfpath[0][pval][cumulative_size[pval]][1] 
						 - oldpvalue[pval][1];

		
			newpvalue[pval] += increment;

			pupdatevals[0][pval] = newpvalue[pval];

			if(time >= pfpath[0][pval][cumulative_size[pval]][0])
			{
				pupdatevals[0][pval] = 
                               pfpath[0][pval][cumulative_size[pval]][1];

				oldpvalue[pval][0] = pfpath[0][pval][cumulative_size[pval]][0];

				pfpathcounter[pval]++; 
				cumulative_size[pval]++;
				newpvalue[pval] = pupdatevals[0][pval];
				oldpvalue[pval][1] = newpvalue[pval];
			
			}
		}
	}
	else
	{
//		printf("parraysize[%i] = %i\n", tag, parray_size[tag][pval][j]);
//		printf("pfpathcounter = %i\n", pfpathcounter[pval]);
//		printf("cumulative_size = %i\n", cumulative_size[pval]);
//		printf("gen_type = %i\n", gen_type[tag][pval][j]);
//	    printf("parray_size[%i] = %i\n", tag, parray_size[tag][pval][j]);
		if(parray_size[tag][pval][j] > pfpathcounter[pval] + 1)
		{
			
			ptables[pval] = ploc(gen_type[tag][pval][j]);
			
			time = cursamp / SR + start;
//			printf("time = %f\n", time);
//			printf("first time = %f\n"
//                 , pfpath[tag][pval][cumulative_size[pval]][0]);
//			printf("second time = %f\n"
//				, pfpath[tag][pval][cumulative_size[pval] + 1][0]);
			if(time >= pfpath[tag][pval][cumulative_size[pval]][0]) 
			{
				if(time >= pfpath[tag][pval][cumulative_size[pval] + 1][0])
				{
					pfpathcounter[pval]++;
					cumulative_size[pval]++;
					oldsamp[pval] = cursamp;
					if(parray_size[tag][pval][j] <= cumulative_size[pval] + 1)
						return;  // should this be tag instead of '0'?
				}

				diff = pfpath[tag][pval][cumulative_size[pval] + 1][1] 
                   - pfpath[tag][pval][cumulative_size[pval]][1];

				difftime = pfpath[tag][pval][cumulative_size[pval] + 1][0] 
                           - pfpath[tag][pval][cumulative_size[pval]][0];

				tableset(difftime, psize(gen_type[tag][pval][j]), ptabs[pval]);

				table_val = tablei(cursamp-oldsamp[pval], ptables[pval]
                                   , ptabs[pval]);

				pupdatevals[tag][pval] = 
             pfpath[tag][pval][cumulative_size[pval]][1] + (table_val * diff);

				printf("table result [%i] = %f\n"
                       , tag, pupdatevals[tag][pval]);
				
			}
		} 
		else
		{
			if(j < numcalls[tag][pval] - 1)
				incr_j = 1;
		}
		if(parray_size[0][pval][j] > pfpathcounter[pval] + 1)
		{
			ptables[pval] = ploc(gen_type[0][pval][j]);
			
			   

			time = cursamp / SR + start;

			if(time >= pfpath[0][pval][cumulative_size[pval]][0])
			{
				if(time >= pfpath[0][pval][cumulative_size[pval] + 1][0])
				{
					pfpathcounter[pval]++;
					cumulative_size[pval]++;
					oldsamp[pval] = cursamp;
					if(parray_size[0][pval][j] <= pfpathcounter[pval] + 1)
						return;
				}
				
				diff = pfpath[0][pval][cumulative_size[pval] + 1][1] 
                   - pfpath[0][pval][cumulative_size[pval]][1];

				difftime = pfpath[0][pval][cumulative_size[pval] + 1][0] 
                           - pfpath[0][pval][cumulative_size[pval]][0];

				tableset(difftime, psize(gen_type[0][pval][j]), ptabs[pval]);
				table_val = tablei(cursamp - oldsamp[pval], ptables[pval]
                                   , ptabs[pval]);

				pupdatevals[0][pval] = 
                                   pfpath[0][pval][cumulative_size[pval]][1] 
                                   + (table_val * diff);

//				printf("diff = %f\n", diff);
//				printf("difftime = %f\n", difftime);
//				printf("table result [%i] = %f\n", cursamp, table_val);
				printf("result [%i] = %f\n", cursamp
                       , pupdatevals[0][pval]);

				
			}
	    }
		else
		{
			if(j < numcalls[0][pval] - 1)
				incr_j = 1;
		}
		
/*		printf("ptabs[%i] = %f\t%f\n", pval, ptabs[pval][0], ptabs[pval][1]);
	    for(i = 0; i < psize(gen_type[tag][pval]); ++i)
	    {
		    printf("ptables[%i] = %f\n", i, ptables[pval][i]);
		}*/
		
	}
	if(incr_j == 1)
	{
		j++;
		cumulative_size[pval]++;
		incr_j = 0;
		pfpathcounter[pval] = 0;
		printf("j = %i\n", j);
			
	}
}

float Instrument::rtupdate(int tag, int pval)
{
  float tval;
  pf_path_update(tag, pval);
  pthread_mutex_lock(&pfieldLock);
  if (pupdatevals[0][pval] != NOPUPDATE) // global takes precedence
  {
	  pthread_mutex_unlock(&pfieldLock);
	  return pupdatevals[0][pval];
  }
  else
	  tval = pupdatevals[tag][pval];
  pupdatevals[tag][pval] = NOPUPDATE;
  pthread_mutex_unlock(&pfieldLock);
  return tval;
}

//		printf("increment = %f\n", increment);
//		printf("oldpvaluse = %f\n", oldpvalue[pval]);
//		printf("first inc = %f\n", increment);
//		printf("updates_per_second= %f\n", updates_per_second);
//		printf("restval = : %i\n", resetval);
//		printf("sample rate =: %f\n", SR);
//		printf("newpvalue = %f\n", newpvalue[pval]);
//		printf("pupdatevals[0][pval] = %f\n", pupdatevals[0][pval]);
//printf("pfpathcounter = %i\n", pfpathcounter[pval]);
