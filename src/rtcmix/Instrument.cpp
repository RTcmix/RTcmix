#include <pthread.h>
#include <iostream.h>
#include "Instrument.h"
#include "rt.h"
#include "rtdefs.h"
#include "../Minc/notetags.h"
#ifdef USE_SNDLIB
#include "../sndlib/sndlib.h"
#endif

#ifdef DEBUG
#include <stdio.h>
#endif

extern int setinput(float, Instrument*);
extern InputDesc inputFileTable[];
extern int rtInteractive;
extern pthread_mutex_t pfieldLock;

Instrument::Instrument()
{
  int i;

  fileOffset = 0;
  fdIndex = -1;
  sfile_on = 0; // default is no input soundfile
  start = 0.0;
  dur = 0.0;
  cursamp = 0;
  endsamp = 0;
  chunkstart = 0;
  chunksamps = 0;
  
  if (tags_on) {
    pthread_mutex_lock(&pfieldLock);
    for (i = 0; i < MAXPUPS; i++) // initialize this element
      pupdatevals[curtag][i] = NOPUPDATE;
    mytag = curtag++;
    if (curtag >= MAXPUPARR) curtag = 1; // wrap it around
    // 0 is reserved for all-note rtupdates
    pthread_mutex_unlock(&pfieldLock);
  }
}

Instrument::~Instrument()
{ 
	if (sfile_on)
		gone(); // decrement input soundfile reference
}

int Instrument::init(float p[], short n_args)
{
	cout << "you haven't defined an init member of your Instrument class!" << endl;
	return -1;
}

int Instrument::run()
{
	cout << "you haven't defined a run member of your Instrument class!" << endl;
	return -1;
}

float Instrument::getstart()
{
	return start;
}

float Instrument::getdur()
{
	return dur;
}

int Instrument::getendsamp()
{
	return endsamp;
}

void Instrument::setchunk(int csamps)
{
  chunksamps = csamps;
}

void Instrument::setchunkstart(int csamps)
{
	chunkstart = csamps;
}

void Instrument::setendsamp(int end)
{
	endsamp = end;
}

void Instrument::gone()
{
  // If the reference count on the file referenced by the instrument
  // reaches zero, close the input soundfile and set the state to 
  // make sure this is obvious

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

#ifdef USE_SNDLIB
	clm_close(inputFileTable[fdIndex].fd);
#else /* !USE_SNDLIB */
  #ifdef sgi
	// free the file handle opened for this file
	afCloseFile((AFfilehandle) inputFileTable[fdIndex].handle);
  #else
	close(inputFileTable[fdIndex].fd);
  #endif
#endif /* !USE_SNDLIB */
      }
      inputFileTable[fdIndex].fd = 0;
#ifdef USE_SNDLIB
      inputFileTable[fdIndex].header_type = unsupported_sound_file;
      inputFileTable[fdIndex].data_format = snd_unsupported;
#else /* !USE_SNDLIB */
  #ifdef sgi
      inputFileTable[fdIndex].handle = NULL;
  #endif
#endif /* !USE_SNDLIB */
      inputFileTable[fdIndex].data_location = 0;
      inputFileTable[fdIndex].dur = 0.0;
      inputFileTable[fdIndex].filename[0] = '\0';
      fdIndex = -1;
    }
  }
}



