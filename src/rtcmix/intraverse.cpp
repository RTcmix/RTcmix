/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <pthread.h>
#include <iostream.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>           // DT: 3/97 needed for time function
#include "../rtstuff/heap/heap.h"
#include "../rtstuff/rtdefs.h"
#include "../H/dbug.h"

#define TBUG
#define ALLBUG

extern "C" {
  void rtsendsamps(void);
  void rtsendzeros(int);
  void rtreportstats(void);
  int rtcloseout(void);
  void rtgetsamps(void);         // DT:  for use with real-time audio input
  void close_audio_ports(void);
}

double baseTime;
long elapsed;

IBusClass checkClass(BusSlot *slot) {
  if (slot == NULL)
	return UNKNOWN;
  if ((slot->auxin_count > 0) && (slot->auxout_count > 0))
	return AUX_TO_AUX;
  if ((slot->auxout_count > 0) && (slot->out_count > 0))
	return TO_AUX_AND_OUT;
  if (slot->auxout_count > 0)
	return TO_AUX;
  if (slot->out_count > 0)
	return TO_OUT;
  return UNKNOWN;
}

extern "C" {
  void *inTraverse()
  {
    short rtInst;
    short playEm;
    int i,j,chunksamps;
    int heapSize,rtQSize,allQSize;
    int offset,endsamp;
    int keepGoing;
    int dummy;
	short bus_q_offset;

    Instrument *Iptr;

    unsigned long bufEndSamp;
    unsigned long chunkStart;
    unsigned long heapChunkStart;

    struct timeval tv;
    struct timezone tz;
    double sec,usec;

	Bool aux_pb_done,frame_done;
	short bus,bus_count,play_bus,busq,endbus;
	IBusClass bus_class,qStatus;
	BusType bus_type;
	
    // cout << "ENTERING inTraverse() FUNCTION *****\n";

    // Wait for the ok to go ahead
    if (!audio_config) {
      cout << "inTraverse():  waiting for audio_config . . . ";
    }

    while (!audio_config) {
      // Do nothing
    }

    if (audio_config && rtInteractive) {
      cout << "audio set.\n\n";
    }

    gettimeofday(&tv, &tz);
    sec = (double)tv.tv_sec;
    usec = (double)tv.tv_usec;
    baseTime = (sec * 1e6) + usec;

    // Initialize everything

    bufStartSamp = 0;  // current end sample for buffer
    bufEndSamp = RTBUFSAMPS;
    chunkStart = 0;
    heapSize = 0;
    chunkStart = 0;
    elapsed = 0;
    rtInst = 0;
    playEm = 0;

    // printf("ENTERING inTraverse() FUNCTION\n");

    // NOTE: audioin, aux and output buffers are zero'd during allocation

    // read in an input buffer (if audio input is active)
    if (audio_on) {
      rtgetsamps();
      rtsendzeros(0);  // send a buffer of zeros to audio device
    }
  
    if (rtsetparams_called)         // otherwise, disk-based only
      playEm = 1;

    while(playEm) { // the big loop ==========================================

      pthread_mutex_lock(&heapLock);
      heapSize = rtHeap.getSize();
      if (heapSize > 0) {
		heapChunkStart = rtHeap.getTop();
      }
      pthread_mutex_unlock(&heapLock);

      // Pop elements off rtHeap and insert into rtQueue ----------------------
      while ((heapChunkStart < bufEndSamp) && (heapSize > 0)) {
        rtInst = 1;
        pthread_mutex_lock(&heapLock);
		Iptr = rtHeap.deleteMin();  // get next instrument off heap
		pthread_mutex_unlock(&heapLock);
		if (!Iptr)
		  break;

		// DJT Now we push things onto different queues
		bus_class = checkClass(Iptr->bus_config);
		switch (bus_class) {
		case TO_AUX:
		  bus_count = Iptr->bus_config->auxout_count;
		  bus_q_offset = 0;
		  for(i=0;i<bus_count;i++) {
			bus = Iptr->bus_config->auxout[i];
			busq = bus+bus_q_offset;
#ifdef TBUG
			cout << "Pushing on TO_AUX[" << busq << "] rtQueue\n";
#endif
			rtQueue[busq].push(Iptr,heapChunkStart);
		  }
		  break;
		case AUX_TO_AUX:
		  bus_count = Iptr->bus_config->auxout_count;
		  bus_q_offset = MAXBUS;
		  for(i=0;i<bus_count;i++) {
			bus = Iptr->bus_config->auxout[i];
			busq = bus+bus_q_offset;
#ifdef TBUG
			cout << "Pushing on AUX_TO_AUX[" << busq << "] rtQueue\n";
#endif
			rtQueue[busq].push(Iptr,heapChunkStart);
		  }
		  break;
		case TO_OUT:
		  bus_count = Iptr->bus_config->out_count;
		  bus_q_offset = MAXBUS*2;
		  for(i=0;i<bus_count;i++) {
			bus = Iptr->bus_config->out[i];
			busq = bus+bus_q_offset;
#ifdef TBUG
			cout << "Pushing on TO_OUT[" << busq << "] rtQueue\n";
#endif
			rtQueue[busq].push(Iptr,heapChunkStart);
		  }
		  break;
		case TO_AUX_AND_OUT:
		  bus_count = Iptr->bus_config->out_count;
		  bus_q_offset = MAXBUS;
		  for(i=0;i<bus_count;i++) {
			bus = Iptr->bus_config->out[i];
			busq = bus+bus_q_offset;
#ifdef TBUG
			cout << "Pushing on TO_OUT2[" << busq << "] rtQueue\n";
#endif
			rtQueue[busq].push(Iptr,heapChunkStart);
		  }
		  bus_count = Iptr->bus_config->auxout_count;
		  bus_q_offset = 2*MAXBUS;
		  for(i=0;i<bus_count;i++) {
			bus = Iptr->bus_config->auxout[i];
			busq = bus+bus_q_offset;
#ifdef TBUG
			cout << "Pushing on TO_AUX2[" << busq << "] rtQueue\n";
#endif
			rtQueue[busq].push(Iptr,heapChunkStart);
		  }
		  break;
		default:
		  cout << "ERROR (intraverse): unknown bus_class\n";
		  break;
		}

		pthread_mutex_lock(&heapLock);
		heapSize = rtHeap.getSize();
		if (heapSize > 0)
		  heapChunkStart = rtHeap.getTop();
        pthread_mutex_unlock(&heapLock);
	  }

	  qStatus = TO_AUX;
	  play_bus = 0;
	  aux_pb_done = NO;
	  allQSize = 0;
	  
	  // rtQueue[] playback shuffling ----------------------------------------
	  while (!aux_pb_done) {

		switch (qStatus) {
		case TO_AUX:
		  bus_q_offset = 0;
		  bus_type = BUS_AUX_OUT;
		  bus = ToAuxPlayList[play_bus++];
		  break;
		case AUX_TO_AUX:
		  bus_q_offset = MAXBUS;
		  bus = AuxToAuxPlayList[play_bus++];
		  bus_type = BUS_AUX_OUT;
		  break;
		case TO_OUT:
		  bus_q_offset = MAXBUS*2;
		  bus = ToOutPlayList[play_bus++];
		  bus_type = BUS_OUT;
		  break;
		default:
		  cout << "ERROR (intraverse): unknown bus_class\n";
		  break;
		}

		if (bus != -1)
		  busq = bus+bus_q_offset;
		else
		  busq = bus;
		if (bus == -1) {
		  switch (qStatus) {
		  case TO_AUX:
			qStatus = AUX_TO_AUX;
			play_bus = 0;
			break;
		  case AUX_TO_AUX:
			qStatus = TO_OUT;
			play_bus = 0;
			break;
		  case TO_OUT:
#ifdef TBUG
			cout << "aux_pb_done\n";
#endif
			aux_pb_done = YES;
			break;
		  default:
			cout << "ERROR (intraverse): unknown bus_class\n";
			break;
		  }
		}
#ifdef TBUG
		cout << "bus: " << bus << endl;
		cout << "busq:  " << busq << endl;
#endif
		if (busq != -1) {
		  rtQSize = rtQueue[busq].getSize();
		  if (rtQSize > 0) {
			chunkStart = rtQueue[busq].nextChunk();
		  }
		}
		// Play elements on queue (insert back in if needed) - - - - - - - -
		while ((rtQSize > 0) && (chunkStart < bufEndSamp) && (bus != -1)) {
		  
#ifdef ALLBUG
		  cout << "Begin iteration==========\n";
		  cout << "Q-chunkStart:  " << chunkStart << endl;
		  cout << "bufEndSamp:  " << bufEndSamp << endl;
		  cout << "RTBUFSAMPS:  " << RTBUFSAMPS << endl;
#endif      
		  Iptr = rtQueue[busq].pop();  // get next instrument off queue
		  
		  endsamp = Iptr->getendsamp();
		  
		  // difference in sample start (countdown)
		  offset = chunkStart - bufStartSamp;  
		  
		  if (offset < 0) { // BGG: added this trap for robustness
			cout << "WARNING: the scheduler is behind the queue!" << endl;
			offset = 0;
		  }
		  
		  Iptr->set_output_offset(offset);
		  
		  if (endsamp < bufEndSamp) {  // compute # of samples to write
			chunksamps = endsamp-chunkStart;
		  }
		  else {
			chunksamps = bufEndSamp-chunkStart;
		  }

		  Iptr->setchunk(chunksamps);  // set "chunksamps"		 
#ifdef TBUG
		  cout << "Iptr->exec(" << bus_type << "," << bus << ")\n";
#endif		  
		  Iptr->exec(bus_type, bus);    // write the samples * * * * * * * * * 
		  
		  switch (qStatus) {
		  case TO_AUX:
			bus_count = Iptr->bus_config->auxout_count;
			endbus = ToAuxPlayList[bus_count-1];
			break;
		  case AUX_TO_AUX:
			bus_count = Iptr->bus_config->auxout_count;
			endbus = ToAuxPlayList[bus_count-1];
			break;
		  case TO_OUT:
			bus_count = Iptr->bus_config->out_count;
			endbus = ToOutPlayList[bus_count-1];
			break;
		  default:
			cout << "ERROR (intraverse): unknown bus_class\n";
			break;
		  }

#ifdef TBUG
		  cout << "endbus " << endbus << endl;
#endif
		  // ReQueue or delete - - - - - - - - - - - - - - - - - - -
		  if (endsamp > bufEndSamp) {
#ifdef ALLBUG
			cout << "inTraverse():  re queueing instrument\n";
#endif
			rtQueue[busq].push(Iptr,chunkStart+chunksamps);   // put back onto queue
		  }
		  else if (bus == endbus) {
 			delete Iptr;
 		  }
		  
		  // DJT:  not sure this check before new chunkStart is necessary
		  rtQSize = rtQueue[busq].getSize();
		  if (rtQSize) {
			chunkStart = rtQueue[busq].nextChunk();
			allQSize += rtQSize;
		  }
#ifdef TBUG
		  cout << "rtQSize: " << rtQSize << endl;
		  cout << "chunkStart:  " << chunkStart << endl;
		  cout << "chunksamps:  " << chunksamps << endl;
		  cout << "Iteration done==========\n";
#endif
		}
	  }
	  
	  // Write buf to audio device -------------------------------------------
#ifdef ALLBUG
	  cout << "Writing samples----------\n";
	  cout << "Q-chunkStart:  " << chunkStart << endl;
	  cout << "bufEndSamp:  " << bufEndSamp << endl;
#endif
	  
	  rtsendsamps();
	  
	  // zero the buffers
	  clear_aux_buffers();
	  clear_output_buffers();
	  
	  // read in an input buffer (if audio input is active)
	  if (audio_on) { 
		// cout << "Reading data from audio port\n";
		rtgetsamps();
	  }
	  
	  gettimeofday(&tv, &tz);
	  sec = (double)tv.tv_sec;
	  usec = (double)tv.tv_usec;
	  baseTime = (sec * 1e6) + usec;
	  elapsed += RTBUFSAMPS;	
	  bufStartSamp += RTBUFSAMPS;
	  bufEndSamp += RTBUFSAMPS;
      
      if (!rtInteractive) {  // Ending condition
		if ((heapSize == 0) && (allQSize == 0)) {
#ifdef TBUG
		  cout << "heapSize:  " << heapSize << endl;
		  cout << "rtQSize:  " << rtQSize << endl;
		  cout << "PLAYEM = 0\n";
		  cout << "The end\n\n";
#endif
		  playEm = 0;
		}
      }
    } // end playEm =========================================================
  
	if (rtsetparams_called) {
	  if (play_audio) {             // Play zero'd buffers to avoid clicks
		int count = NCHANS * 4;  // DJT:  clicks on my box with 2
		for (j = 0; j < count; j++) 
		  rtsendzeros(0);
      }  
	  close_audio_ports();
	  rtreportstats();
	  rtcloseout();
	}

	cout << "\n";
	// cout << "EXITING inTraverse() FUNCTION *****\n";
	// exit(1);
  }

} /* extern "C" */
