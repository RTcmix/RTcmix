/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <iostream.h>
#include "Instrument.h"
#include "rt.h"
#include "rtdefs.h"
#include <notetags.h>
#include <sndlibsupport.h>
#include <bus.h>
#include <assert.h>
#include <ugens.h>
#include "heap/heap.h"
#include <PField.h>
#include <PFieldSet.h>

/* ----------------------------------------------------------- Instrument --- */
Instrument::Instrument()
	: _start(0.0), _dur(0.0), cursamp(0), chunksamps(0), i_chunkstart(0),
	  endsamp(0), nsamps(0), output_offset(0), fdIndex(NO_DEVICE_FDINDEX),
	  fileOffset(0), inputsr(0.0), inputchans(0), outputchans(0), _name(NULL)
{
   int i;

#ifdef RTUPDATE
   for(i = 0; i < MAXNUMPARAMS; ++i)
   {
	   pfpathcounter[i] = 0;
	   cumulative_size[i] = 0;
	   newpvalue[i] = 0;
	   oldsamp[i] = -1;
	   j[i] = 0;
	   k[i] = 0;
   }
   _startOffset = 0;
#endif /* RTUPDATE */

   sfile_on = 0;                // default is no input soundfile

   outbuf = NULL;
   _busSlot = NULL;
#ifdef PFIELD_CLASS
   _pfields = NULL;
#endif

   for (i = 0; i < MAXBUS; i++)
      bufstatus[i] = 0;
   needs_to_run = 1;

#ifdef RTUPDATE
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
#endif /* RTUPDATE */
}


/* ---------------------------------------------------------- ~Instrument --- */
Instrument::~Instrument()
{
	if (sfile_on)
		gone();                   // decrement input soundfile reference

	delete [] outbuf;

	RefCounted::unref(_busSlot);	// release our reference	

#ifdef PFIELD_CLASS
	delete _pfields;
#endif	
	delete [] _name;
}

/* ------------------------------------------------------- setName --- */
/* This is only called by set_bus_config
*/

void Instrument::setName(const char *name)
{
	_name = new char[strlen(name) + 1];
	strcpy(_name, name);
}

/* ------------------------------------------------------- set_bus_config --- */
/* Set the _busSlot pointer to the right bus_config for this inst.
   Then set the inputchans and outputchans members accordingly.
  
   Instruments *must* call this from within their makeINSTNAME method. E.g.,
  
       WAVETABLE *inst = new WAVETABLE();
       inst->set_bus_config("WAVETABLE");
*/
void Instrument::set_bus_config(const char *inst_name)
{
  setName(inst_name);

  pthread_mutex_lock(&bus_slot_lock);
  _busSlot = ::get_bus_config(inst_name);
  _busSlot->ref();		// add our reference to this
  
  inputchans = _busSlot->in_count + _busSlot->auxin_count;
  outputchans = _busSlot->out_count + _busSlot->auxout_count;
  pthread_mutex_unlock(&bus_slot_lock);
}

#ifdef PFIELD_CLASS
double Instrument::s_dArray[MAXDISPARGS];
#endif

/* ------------------------------------------------------------ setup () --- */

// This function is now the one called by checkInsts().  It calls init().

int Instrument::setup(PFieldSet *pfields)
{
#ifdef PFIELD_CLASS
	_pfields = pfields;
	int nargs = MAXDISPARGS;
	update(s_dArray, nargs);
	return init(s_dArray, pfields->size());
#else
	return -1;
#endif
}

/* ------------------------------------------------------------ update () --- */

// This function is called during run() by Instruments which want updated
// values for each pfield slot.  'nvalues' is size of p[].
// 'fields' is a bitmask of fields between [0] and [nvalues - 1] to fill.

int Instrument::update(double p[], int nvalues, unsigned fields)
{
#ifdef PFIELD_CLASS
	int n, args = _pfields->size();
	int frame = currentFrame();
	double percent = (frame == 0) ? 0.0 : (double) frame / nSamps();
	if (nvalues < args)
		args = nvalues;
	for (n = 0; n < args; ++n) {
		if (fields & (1 << n))
			p[n] = (*_pfields)[n].doubleValue(percent);
	}
	for (; n < nvalues; ++n)
		p[n] = 0.0;
#endif
	return 0;
}

/* ------------------------------------------------------------ init() --- */

// This function is now called by setup().  When using RTUPDATE, it initializes
// the newpvalue and oldpvalue arrays which are used in the linear 
// interpolation case.  It also identifies the slot number for the instrument.

int Instrument::init(double p[], int n_args)
{
#ifdef RTUPDATE
   int i;

   for(i = 0; i < n_args; ++i)
   {
	   newpvalue[i] = p[i];	
	   oldpvalue[i][0] = 0;
	   oldpvalue[i][1] = p[i];
   }
   slot = piloc(instnum);
   return 777;
#else /* !RTUPDATE */
   cout << "You haven't defined an init member of your Instrument class!"
                                                                 << endl;
   return -1;
#endif /* !RTUPDATE */
}

/* ----------------------------------------------------- configure(int) --- */

// This function performs any internal configuration and allocation needed
// to run the instrument following the call to init() and preceeding the
// first call to run().  It allows the majority of the memory allocation to
// be postponed until the note is run.  
//

/* 
   It is called by the RTcmix system only and never by derived classes.
   This method allocates the instrument's private interleaved output buffer
   and inits a buffer status array, then calls the virtual class-specific 
   configure(void) method.
   Note: We allocate here, rather than in ctor or init method, because this
   will mean less memory overhead before the inst begins playing.
*/

int Instrument::configure(int bufsamps)
{
	assert(outbuf == NULL);	// configure called twice, or recursively??
	outbuf = new BUFTYPE [bufsamps * outputchans];
	return configure();		// Class-specific configuration.
}

/* ----------------------------------------------------- configure(void) --- */

// This is the virtual function that derived classes override.  We supply a
// default base class version because not all subclasses need/use this method.

int Instrument::configure(void)
{
	return 0;	// 0 is success, -1 is failure.
}

/* ------------------------------------------------------------------ run --- */
/* 
   This function is called by the RTcmix system only and never by derived
   classes.  This method calls the virtual class-specific run(void) method.
   Note that Instrument::run() no longer needs to allocate memory.  Now handled
   by Instrument::configure(int).
*/

int Instrument::run(bool needsTo)
{
   if (needsTo) {
	   obufptr = outbuf;

	   for (int i = 0; i < outputchans; i++)
		  bufstatus[i] = 0;

	   needs_to_run = 0;

	   return run();	// Class-specific run().
   }
   return 0;
}

/* ------------------------------------------------------------- schedule --- */
/* Called from checkInsts to place the instrument into the scheduler heap */

void Instrument::schedule(heap *rtHeap)
{
  int nsamps,startsamp,endsamp;
  int heapSize;
  int curslot;
  float start,dur;

  // Calculate variables for heap insertion
  dur = getdur();
  nsamps = (int) (dur*SR);
  //  cout << "nsamps = " << nsamps << endl;
  
  start = getstart();
  startsamp = (int) (start*SR);
  
  if (rtInteractive) {
#ifdef RTUPDATE
	_startOffset = (elapsed + RTBUFSAMPS);
#endif
  	// Adjust start frame based on elapsed frame count
  	startsamp += (elapsed + RTBUFSAMPS);
  }
  
  endsamp = startsamp+nsamps;
  setendsamp(endsamp);  // used by traverse.C
  
  // place instrument into heap
  rtHeap->insert(this, startsamp);
  
  // printf("Instrument::schedule(): %d\n", heapSize);
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

   If we've written (i.e., called addout) all the buses for the current
   chunk, then return 1.  Else, return 0.
*/
int Instrument::exec(BusType bus_type, int bus)
{
   int done;

   run(needs_to_run);	// Only does anything if true.

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

   return (int) needs_to_run;
}


/* ------------------------------------------------------------- rtaddout --- */
/* Replacement for the old rtaddout (in rtaddout.C, now removed).
   This one copies (not adds) into the inst's outbuf. Later the
   scheduler calls the insts addout method to add outbuf into the 
   appropriate output buses. 
   Assumes that <samps> contains at least outputchans interleaved samples.
   Returns outputchans (i.e., number of samples written).
*/
int Instrument::rtaddout(BUFTYPE samps[])
{
   for (int i = 0; i < outputchans; i++)
      *obufptr++ = samps[i];

   return outputchans;
}

/* ------------------------------------------------------------ rtbaddout --- */
/* Block version of rtaddout.  Useful for instruments which generate samples
   in chunks, and can write them to the output buffer in one operation.
*/

int Instrument::rtbaddout(BUFTYPE samps[], int length)
{
	const int sampcount = length * outputchans;
	for (int i = 0; i < sampcount; i++)
		*obufptr++ = samps[i];

	return sampcount;
}


/* --------------------------------------------------------------- addout --- */
/* Add signal from one channel of instrument's private interleaved buffer
   into the specified output bus.
*/
void Instrument::addout(BusType bus_type, int bus)
{
   int      samp_index, endframe, src_chan, buses;
   short    *bus_list;
   BufPtr   src, dest;

   assert(bus >= 0 && bus < MAXBUS);

   if (bus_type == BUS_AUX_OUT) {
      dest = aux_buffer[bus];
      buses = _busSlot->auxout_count;
      bus_list = _busSlot->auxout;
   }
   else {       /* BUS_OUT */
      dest = out_buffer[bus];
      buses = _busSlot->out_count;
      bus_list = _busSlot->out;
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


/* ----------------------------------------------------------- setendsamp --- */
void Instrument::setendsamp(int end)
{
  pthread_mutex_lock(&endsamp_lock);
  endsamp = end;
  pthread_mutex_unlock(&endsamp_lock);
}

/* ----------------------------------------------------------------- gone --- */
/* If the reference count on the file referenced by the instrument
   reaches zero, close the input soundfile and set the state to 
   make sure this is obvious.
*/
void Instrument::gone()
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
            mus_file_close(inputFileTable[fdIndex].fd);
         }
         if (inputFileTable[fdIndex].filename);
            free(inputFileTable[fdIndex].filename);
         inputFileTable[fdIndex].filename = NULL;
         inputFileTable[fdIndex].fd = NO_FD;
         inputFileTable[fdIndex].header_type = MUS_UNSUPPORTED;
         inputFileTable[fdIndex].data_format = MUS_UNSUPPORTED;
         inputFileTable[fdIndex].is_float_format = 0;
         inputFileTable[fdIndex].data_location = 0;
         inputFileTable[fdIndex].srate = 0.0;
         inputFileTable[fdIndex].chans = 0;
         inputFileTable[fdIndex].dur = 0.0;
         fdIndex = NO_DEVICE_FDINDEX;
      }
   }
}

#ifdef RTUPDATE
/* --------------------set_instnum------------------------------------------ */

// This function is used to set up the mapping between instrument names and 
// their associated instrument tag numbers.  This function will set the 
// number for a given instrument, based on the name that it is passed, and 
// will associate the next availiable number with that name, if the name has 
// not already been given a number
void Instrument::set_instnum(char* name)
{
	inst_list *tmp;

	// first inst called so need to allocate memory for the head of the list
	if(ilist == NULL)
	{	
		ilist = (struct inst_list *) malloc(sizeof(struct inst_list));
		ilist->next = NULL;
		ilist->num = curinst++;
	    ilist->name = name;
	}
	tmp = ilist;

	// check to see if the inst already exists in the list and iterates to the
	// end of the list
	while((strcmp(name, tmp->name) != 0) && (tmp->next != NULL))
	{
		tmp = tmp->next;
	}

	// if the instrument already exists assign the instruments tag number to 
	// the pre-existing number already given to that instrument
	if(strcmp(name, tmp->name) == 0)
	{
		instnum = tmp->num;
	}

	// otherwise give it a new tag number and add it to the end of the list
	// (the -1 flag is used in the same way it is in the initial case
	else
	{	  
		tmp->next = (struct inst_list *) malloc(sizeof(struct inst_list));
		tmp = tmp->next;
		tmp->next = NULL;
		tmp->num = curinst++;
		tmp->name = name;
		instnum = tmp->num;
	}
	return;
}

/* --------------------pi_path_update--------------------------------------- */

// This function (called by pf_path_update) is used to test whether valid
// data exists in the pipath array to update the relevant parameter.  If data
// does exist and no data exists for the tag number (instance of an instrument)
// that is calling this function, then this function sets the data in the 
// pfpath array for that tag and parameter to the values in the pipath array
// for that instrument tag and parameter
void Instrument::pi_path_update(int pval)
{
	int i;

	if(slot < 0)
		return;

	// while data exists in the pipath array and data does not exist in the 
	// pfpath array, set the pfpath data to be the pipath data.  The k 
	// iterator is used to distinguish between multiple calls/gen types for
	// the array
	
	while((piarray_size[slot][pval][k[pval]] != 0) 
         && (parray_size[mytag][pval][k[pval]] == 0))
	{

		gen_type[mytag][pval][k[pval]] = igen_type[slot][pval][k[pval]];
		parray_size[mytag][pval][k[pval]] = piarray_size[slot][pval][k[pval]];
		numcalls[mytag][pval] = numinstcalls[slot][pval];
		for(i = 0; i < cum_piarray_size[slot][pval]; ++i)
		{
			pfpath[mytag][pval][i][0] = pipath[slot][pval][i][0];
			pfpath[mytag][pval][i][1] = pipath[slot][pval][i][1];

		}
		k[pval]++;
	}

	return;
}

/* --------------------pf_path_update--------------------------------------- */

// This function is where all of the actual parameter value changing occurs.
// It is called from rtupdate and updates the pupdatevals array to whatever
// the correct value should be.  Note on how to read pfpath array:  the array
// is really an arrangement of sets of time-value pairs.  This function will 
// usually be operating on a "set of time-value pairs".  This means that the 
// program considers time-value pairs in groups of two, where it will 
// interpolate from the "start value" to the "end value" beginning at the 
// "start time" and ending at the "end time".  These four values in quotation
// marks are the four values specified be each "set of time-value pairs".
void Instrument::pf_path_update(int tag, int pval)
{
	float time, increment;
	float updates_per_second; 
	int i, incr_j;
	double table_val, diff, difftime;
	double start_index;
	pi_path_update(pval);
	incr_j = 0;

	// if the user specifies gen type '0' use a linear interpolation.  Said
	// interpolation is defined in the body of this if statement
	if(gen_type[tag][pval][j[pval]] == 0 && gen_type[0][pval][j[pval]] == 0) 
	{
		// pfpathcounter is used to keep track of which time-values pair 
		// the program is currently considering, if this is greater than the 
		// total number of time value pairs for a given call (specified by 
		// parray_size...with j representing which call/gen to use) than go 
		// on to the next call
		if(parray_size[tag][pval][j[pval]] > pfpathcounter[pval])
		{
			time = (cursamp - _startOffset) / SR + _start;

			// this statement insures that time 0 = "now" for the real time
			// performance case
// DS			time -= schedtime;

			if(time < pfpath[tag][pval][0][0]) // before inst reaches first 
				return;						   // time specified in pfpath


			updates_per_second = resetval;
			
			// increment equals the reciprical of the difference in time 
			// between the current time value that you're working on (pfpath[0]
            // and the last time value that you are working on. (oldpvalue[0])
			// multiply this by the number of times this function gets called
			// per second to determine how long each block of time is.  This
			// is then multiplied by the difference between the current 
			// parameter value (pfpath[1]) and the old parameter value
			// (oldpvalue[1])
			increment = 1 / ((pfpath[tag][pval][cumulative_size[pval]][0] 
							 - oldpvalue[pval][0]) * updates_per_second);

			increment *= pfpath[tag][pval][cumulative_size[pval]][1] 
						 - oldpvalue[pval][1];

		
			// the new value is then incremented...this creates a linear 
			// interpolation
			newpvalue[pval] += increment;

			pupdatevals[tag][pval] = newpvalue[pval];

			// if you've passed the time specified in the pfpath array go on
			// to the next time value pair and update pfpathcounter, 
			// cumulative_size, newpvalue, and oldpvalue
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

		// now do the same actions as above, only test it for the global case.
		// this is important because the user can specify information for a 
		// given note tag or for all notes.  Therefore the global array must
		// be checked
		if(parray_size[0][pval][j[pval]] > pfpathcounter[pval])
		{		
			time = (cursamp - _startOffset) / SR + _start;

			// this statement insures that time 0 = "now" for the real time
			// performance case
// DS			time -= schedtime;

			if(time < pfpath[tag][pval][0][0]) // before inst reaches first 
				return;						   // time specified in pfpath

			updates_per_second = resetval;


			// increment equals the reciprical of the difference in time 
			// between the current time value that you're working on (pfpath[0]
            // and the last time value that you are working on. (oldpvalue[0])
			// multiply this by the number of times this function gets called
			// per second to determine how long each block of time is.  This
			// is then multiplied by the difference between the current 
			// parameter value (pfpath[1]) and the old parameter value
			// (oldpvalue[1])
			increment = 1 / ((pfpath[0][pval][cumulative_size[pval]][0] 
							 - oldpvalue[pval][0]) * updates_per_second);

			increment *= pfpath[0][pval][cumulative_size[pval]][1] 
						 - oldpvalue[pval][1];

		    // the new value is then incremented...this creates a linear 
			// interpolation
			newpvalue[pval] += increment;

			pupdatevals[0][pval] = newpvalue[pval];


			// if you've passed the time specified in the pfpath array go on
			// to the next time value pair and update pfpathcounter, 
			// cumulative_size, newpvalue, and oldpvalue
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
	// This case deals with a user specified pgen for interpolation
	else
	{
		// pfpathcounter is used to keep track of which time-values pair 
		// the program is currently considering, if this is greater than the 
		// total number of time value pairs for a given call (specified by 
		// parray_size...with j representing which call/gen to use) than go 
		// on to the next call
		if(parray_size[tag][pval][j[pval]] > pfpathcounter[pval] + 1)
		{
			ptables[pval] = ploc(gen_type[tag][pval][j[pval]]);
			
			time = (cursamp - _startOffset) / SR + _start;
// DS			time -= schedtime;

			// cumulative size is storing the current index into the pfpath 
            // array so this statement is testing to see if the current 
            // time is greater than the start time of the current set of 
            // time-value pairs
		  
			if(time >= pfpath[tag][pval][cumulative_size[pval]][0]) 
			{
				// if the current time is greater than the end time of the 
				// current set of time-value pairs, than update the counters
				// that dictate which set of time-value pairs the program is
				// currently working on
				if(time >= pfpath[tag][pval][cumulative_size[pval] + 1][0])
				{
					pfpathcounter[pval]++;
					cumulative_size[pval]++;
					oldsamp[pval] = cursamp;

					// this test prevents the program from accessing data from
					// outside the bounds of the array by verifying that 
					// cumulative_size[pval] is not currently indexing the last
					// valid element in the array
					if(parray_size[tag][pval][j[pval]] 
                                     <= cumulative_size[pval] + 1)
						return;  
				}
				
				// diff is the difference between the "end value" and the 
				// "start value"
				diff = pfpath[tag][pval][cumulative_size[pval] + 1][1] 
                   - pfpath[tag][pval][cumulative_size[pval]][1];

				// difftime is the difference between the "end time" and the
				// "start time"
				difftime = pfpath[tag][pval][cumulative_size[pval] + 1][0] 
                           - pfpath[tag][pval][cumulative_size[pval]][0];

				
				tableset(difftime, psize(gen_type[tag][pval][j[pval]])
                                 , ptabs[pval]);
				

				// This flag is used to test if this is the first time that 
				// a value update will occur.  For the first update 
                // cursamp - oldsamp[pval] should equal whatever the desired 
				// first index into the table should be.  This is determined by
				// the percentage of time that has elapsed in the current 
				// interpolation multiplied by the total size of the table
				// (SR * difftime)
				if(oldsamp[pval] == -1)
				{
					oldsamp[pval] = cursamp;
					start_index = (time 
                                 - pfpath[tag][pval][cumulative_size[pval]][0])
								 / difftime;
					start_index *= SR * difftime;
					oldsamp[pval] -= (int)start_index;
				}
				table_val = tablei(cursamp - oldsamp[pval], ptables[pval]
                                   , ptabs[pval]);

				pupdatevals[tag][pval] = 
             pfpath[tag][pval][cumulative_size[pval]][1] + (table_val * diff);

			}
		} 

		// if the array is not bigger than pfpathcounter and there are still
		// other valid data in the pfpath array than increment 'j' to move
		// to the next set of data
		else
		{
			if(j[pval] < numcalls[tag][pval] - 1)
				incr_j = 1;
		}

		// now do the same actions as above, only test it for the global case.
		// this is important because the user can specify information for a 
		// given note tag or for all notes.  Therefore the global array must
		// be checked
		if(parray_size[0][pval][j[pval]] > pfpathcounter[pval] + 1)
		{
			ptables[pval] = ploc(gen_type[0][pval][j[pval]]);
			
			   

			time = (cursamp - _startOffset) / SR + _start;

			// this statement insures that time 0 = "now" for the real time
			// performance case
// DS			time -= schedtime;

			// cumulative size is storing the current index into the pfpath 
            // array so this statement is testing to see if the current 
            // time is greater than the start time of the current set of 
            // time-value pairs
			if(time >= pfpath[0][pval][cumulative_size[pval]][0])
			{
				// if the current time is greater than the end time of the 
				// current set of time-value pairs, than update the counters
				// that dictate which set of time-value pairs the program is
				// currently working on
				if(time >= pfpath[0][pval][cumulative_size[pval] + 1][0])
				{
					pfpathcounter[pval]++;
					cumulative_size[pval]++;
					oldsamp[pval] = cursamp;

					// this test prevents the program from accessing data from
					// outside the bounds of the array by verifying that 
					// cumulative_size[pval] is not currently indexing the last
					// valid element in the array
					if(parray_size[0][pval][j[pval]] 
                                 <= pfpathcounter[pval] + 1)

						return;
				}

				// diff is the difference between the "end value" and the 
				// "start value"
				diff = pfpath[0][pval][cumulative_size[pval] + 1][1] 
                   - pfpath[0][pval][cumulative_size[pval]][1];
				
				// difftime is the difference between the "end time" and the
				// "start time"
				difftime = pfpath[0][pval][cumulative_size[pval] + 1][0] 
                           - pfpath[0][pval][cumulative_size[pval]][0];

				tableset(difftime, psize(gen_type[0][pval][j[pval]])
                                                 , ptabs[pval]);


				// This flag is used to test if this is the first time that 
				// a value update will occur.  For the first update 
                // cursamp - oldsamp[pval] should equal whatever the desired 
				// first index into the table should be.  This is determined by
				// the percentage of time that has elapsed in the current 
				// interpolation multiplied by the total size of the table
				// (SR * difftime)
				if(oldsamp[pval] == -1)
				{
					oldsamp[pval] = cursamp;
					start_index = (time 
                                 - pfpath[0][pval][cumulative_size[pval]][0])
								 / difftime;
					start_index *= SR * difftime;
					oldsamp[pval] -= (int)start_index;
				}

				table_val = tablei(cursamp - oldsamp[pval], ptables[pval]
                                   , ptabs[pval]);

				pupdatevals[0][pval] = 
                                   pfpath[0][pval][cumulative_size[pval]][1] 
                                   + (table_val * diff);

			}
	    }

		// if the array is not bigger than pfpathcounter and there are still
		// other valid data in the pfpath array than increment 'j' to move
		// to the next set of data
		else
		{
			if(j[pval] < numcalls[0][pval] - 1)
				incr_j = 1;
		}
	}

	// increment the values involved in moving on to the next set of data
	if(incr_j == 1)
	{
		j[pval]++;
		cumulative_size[pval]++;
		incr_j = 0;
		pfpathcounter[pval] = 0;
		oldsamp[pval] = -1;
	}
}

/* --------------------rtupdate-------------------------------------------- */

// This function is called by the instruments to check to see if there is a 
// parameter update.  It calls pf_path_update to see if there is update data
// specified by the score file, but it could also receive socket update data
// if it is sent
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

// Rise, Sustain, Decay code + + + + + + + + + + + + + + + + 
void Instrument::RSD_setup(int RISE_SLOT, int SUSTAIN_SLOT, int DECAY_SLOT
                           , float duration)
{
	rsd_env = NONE;
	rise_samps = sustain_samps = decay_samps = 0;
	
	rise_table = sustain_table = decay_table = NULL;

	
	if((sustain_time || decay_time) && (rise_time <= 0))
		rise_time = 0.0001;
	if(rise_time)
	{
		rise_table = floc(RISE_SLOT);
		if(rise_table)
		{
			tableset(rise_time, fsize(RISE_SLOT), r_tabs);
			rise_samps = (int)(rise_time * SR);
			rsd_env = RISE;
		}
	}

	if((rise_time || decay_time) && (sustain_time <= 0))
		sustain_time = 0.0001;
	if(sustain_time)
	{
		sustain_table = floc(SUSTAIN_SLOT);
		if(sustain_table)
		{
			tableset(sustain_time, fsize(SUSTAIN_SLOT), s_tabs);
			sustain_samps = (int)(sustain_time * SR);
		}
	}

	if((rise_time || sustain_time) && (decay_time <= 0))
		decay_time = 0.0001;
	if(decay_time)
	{
		decay_table = floc(DECAY_SLOT);
		if(decay_table)
		{
			tableset(decay_time, fsize(DECAY_SLOT), d_tabs);
			decay_samps = (int)(decay_time * SR);
		}
	}

	if((rise_table) || (sustain_table) || (decay_table))
	{
		_dur = (rise_time + sustain_time + decay_time);
	}
	else
		_dur = duration;
	return;
}

	// End RSD Setup - - - - - - - - - - - - - - - - - - - - -

// Check RSD Envelope + + + + + + + + + + + + + + + + + + + + 
void Instrument::RSD_check()
{
	if((rsd_samp > rise_samps) && (rsd_env == RISE))
	{
		rsd_env = SUSTAIN;
		rsd_samp = 0;
	}
	else if((rsd_samp > sustain_samps) && (rsd_env == SUSTAIN))
	{
		rsd_env = DECAY;
		rsd_samp = 0;
	}
	return;
}

// End RSD Envelope Check - - - - - - - - - - - - - - - -

// Check which Amplitude Envelope to use + + + + + + +
float Instrument::RSD_get()
{
	float retvalue;
	if(rsd_env == RISE)
	{
		retvalue = tablei(rsd_samp, rise_table, r_tabs);
	}
	else if(rsd_env == SUSTAIN)
	{
		retvalue = tablei(rsd_samp, sustain_table, s_tabs);
	}
	else if(rsd_env == DECAY)
	{
		retvalue = tablei(rsd_samp, decay_table, d_tabs);
	}
	else
	{
		retvalue = -1;
	}
	return retvalue;
}
// End Amp Env Check - - - - - - - - - - - - - - - - - 

#endif /* RTUPDATE */
