/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <pthread.h>
#include <iostream.h>
#include "../rtstuff/heap/heap.h"
#include "../rtstuff/rtdefs.h"
#include "../H/byte_routines.h"
#include "../H/dbug.h"

extern "C" {
#include <sys/time.h>           // DT: 3/97 needed for time function
#include <unistd.h>
#include <stdio.h>
  void *rtsendsamps(short[]);
  short *rtrescale(float[]);
  void rtreportstats(void);
#ifdef USE_SNDLIB
  int rtwritesamps(short[]);
  int rtcloseout();
#endif
  int rtgetsamps();            // DT:  for use with real-time audio input
}

double baseTime;
long elapsed;

IBusClass checkClass(BusSlot *slot) {
  if ((slot->auxin_count > 0) && (slot->auxout_count > 0))
	return AUX_TO_AUX;
  if (slot->auxout_count > 0)
	return TO_AUX;
  return TO_OUT;
}

extern "C" {
  void *inTraverse()
  {
    short rtInst;
    short playEm;
    int i,j,chunksamps;
    int heapSize,rtQSize;
    int offset,endsamp;
    int keepGoing;
    int dummy;
    short *sbuf;

    Instrument *Iptr;

    unsigned long bufEndSamp;
    unsigned long chunkStart;
    unsigned long heapChunkStart;

    struct timeval tv;
    struct timezone tz;
    double sec,usec;

	Bool aux_pb_done;
	short bus,bus_count,play_bus;
	IBusClass bus_class,qStatus;
	
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

    // zero the output audio buffer
    if (out_port) {
      for (j = 0; j < (RTBUFSAMPS*NCHANS); j++) {
		outbuff[j] = 0.0;
		inbuff[j] = 0;
      }
    }

    // read in an input buffer (if audio input is active)
    if (audio_on) {
      outbptr = &outbuff[0];  // advance buffer pointer
      rtgetsamps();
      sbuf = rtrescale(outbuff);
      rtsendsamps(sbuf);  // send a buffer of zeros
    }
  
    bufStartSamp = 0;  // current end sample for buffer
    bufEndSamp = RTBUFSAMPS;
    chunkStart = 0;

    if (out_port)
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
		Iptr->setchunkstart(heapChunkStart);
		bus_class = checkClass(Iptr->bus_config);
		switch (bus_class) {
		case TO_AUX:
		  rtQueue[MAXBUS+1].push(Iptr);
		  break;
		case AUX_TO_AUX:
		  bus_count = Iptr->bus_config->auxout_count;
		  for(i=0;i<bus_count;i++) {
			bus = Iptr->bus_config->auxout[i];
			rtQueue[bus].push(Iptr);
		  }
		  break;
		case TO_OUT:
		  rtQueue[MAXBUS+2].push(Iptr);
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
	  
	  // rtQueue[] playback shuffling ----------------------------------------
	  while (!aux_pb_done) {

		switch (qStatus) {
		case TO_AUX:
		  bus = MAXBUS+1;
		  break;
		case AUX_TO_AUX:
		  bus = AuxPlayList[play_bus];
		  break;
		case TO_OUT:
		  bus = MAXBUS+2;
		  break;
		default:
		  cout << "ERROR (intraverse): unknown bus_class\n";
		  break;
		}

		rtQSize = rtQueue[bus].getSize();
		if (rtQSize > 0)
		  chunkStart = rtQueue[bus].nextChunk();

		// Play elements on queue (insert back in if needed) - - - - - - - -
		while ((rtQSize > 0) && (chunkStart < bufEndSamp)) {
		  
#ifdef ALLBUG
		  cout << "Q-chunkStart:  " << chunkStart << endl;
		  cout << "bufEndSamp:  " << bufEndSamp << endl;
		  cout << "RTBUFSAMPS:  " << RTBUFSAMPS << endl;
#endif      
		  Iptr = rtQueue[bus].pop();  // get next instrument off queue
		  
		  endsamp = Iptr->getendsamp();
		  
		  // difference in sample start (countdown)
		  offset = (chunkStart-bufStartSamp)*NCHANS;  
		  
		  if (offset < 0) { // BGG: added this trap for robustness
			cout << "WARNING: the scheduler is behind the queue!" << endl;
			offset = 0;
		  }
		  
		  outbptr = &outbuff[offset];  // advance buffer pointer
		  
		  if (endsamp < bufEndSamp) {  // compute # of samples to write
			chunksamps = endsamp-chunkStart;
		  }
		  else {
			chunksamps = bufEndSamp-chunkStart;
		  }
		  
		  Iptr->setchunk(chunksamps);  // set "chunksamps"
		  
		  Iptr->run();    // write the samples * * * * * * * * * * * 
		  
		  // ReQueue or delete - - - - - - - - - - - - - - - - - - -
		  if (endsamp > bufEndSamp) {
			Iptr->setchunkstart(chunkStart+chunksamps);  // reset chunkStart
#ifdef ALLBUG
			cout << "inTraverse():  re queueing instrument\n";
#endif
			rtQueue[bus].push(Iptr);   // put back onto queue
		  }
		  else {
			delete Iptr;
		  }
		  
		  // DJT:  not sure this check before new chunkStart is necessary
		  rtQSize = rtQueue[bus].getSize();
		  if (rtQSize)
			chunkStart = rtQueue[bus].nextChunk();
		}
		
		switch (qStatus) {
		case TO_AUX:
		  qStatus = AUX_TO_AUX;
		  break;
		case AUX_TO_AUX:
		  play_bus++;
		  bus = AuxPlayList[play_bus];
		  if (bus == -1) 
			qStatus = TO_OUT;
		  break;
		case TO_OUT:
		  aux_pb_done = YES;
		  break;
		default:
		  cout << "ERROR (intraverse): unknown bus_class\n";
		  break;
		}
	  }
	  
	  // Write buf to audio device -------------------------------------------
	  if (chunkStart >= bufEndSamp) {  
#ifdef ALLBUG
		cout << "Writing samples\n";
		cout << "Q-chunkStart:  " << chunkStart << endl;
		cout << "bufEndSamp:  " << bufEndSamp << endl;
#endif
		
		// cout << "play_audio = " << play_audio << endl;
		
		sbuf = rtrescale(outbuff);
		
		if (play_audio)
		  rtsendsamps(sbuf);
		
		// Write audio buffer to file
		if (rtfileit) {
#ifdef USE_SNDLIB
		  rtwritesamps(sbuf);
#else
		  if (rtoutswap) {
			for (j = 0; j < MAXBUF; j++) {
			  byte_reverse2(&sbuf[j]);
			}
		  }
		  // NOTE: old way doesn't support writing to float files
		  if (write(rtoutfile, sbuf, sizeof(short)*NCHANS*RTBUFSAMPS) == -1) {
			fprintf(stderr, "intraverse():  bad write to audio file\n");
		  } 
#endif // USE_SNDLIB
		}      
		
		gettimeofday(&tv, &tz);
		sec = (double)tv.tv_sec;
		usec = (double)tv.tv_usec;
		baseTime = (sec * 1e6) + usec;
		elapsed += RTBUFSAMPS;	
		bufStartSamp += RTBUFSAMPS;
		bufEndSamp += RTBUFSAMPS;
		
		// zero the output audio buffer
		for (j = 0; j < (RTBUFSAMPS*NCHANS); j++) {
		  outbuff[j] = 0.0;
		  inbuff[j] = 0;
		}
		
		// read in an input buffer (if audio input is active)
		if (audio_on) { 
		  // cout << "Reading data from audio port\n";
		  rtgetsamps();
		}
	  }
      
	  // Some checks for the next case v v v v v v v v v v v v
	  pthread_mutex_lock(&heapLock);
	  heapSize = rtHeap.getSize();
	  if (heapSize > 0) {
		heapChunkStart = rtHeap.getTop();
	  }
	  pthread_mutex_unlock(&heapLock);
	  // DJT: this might be unecessary
      rtQSize = rtQueue[bus].getSize();
	  
	  // Nothing on the queue and nothing on the heap for playing -------------
	  // write zeros
	  if (!(rtQSize) && (heapChunkStart > bufEndSamp)) {
		
		sbuf = rtrescale(outbuff);
		if (play_audio && rtInst) {
		  rtsendsamps(sbuf);
		}
		
		// Write audio buffer to file
		// ***FIXME: this writes extra MAXBUF zeros to end of file
		// ***need to make intraverse aware of what it's doing (e.g, server?)
		if (rtfileit && rtInst) {
#ifdef USE_SNDLIB
		  rtwritesamps(sbuf);
#else
		  if (write(rtoutfile, sbuf, sizeof(short)*NCHANS*RTBUFSAMPS) == -1) {
			fprintf(stderr, "intraverse():  bad write to audio file\n");
		  } 
#endif // !USE_SNDLIB
		}
		
		// Increment time
		gettimeofday(&tv, &tz);
		sec = (double)tv.tv_sec;
		usec = (double)tv.tv_usec;
		baseTime = (sec * 1e6) + usec;
		elapsed += RTBUFSAMPS;
		bufStartSamp += RTBUFSAMPS;
		bufEndSamp += RTBUFSAMPS;
      
		// zero the output audio buffer
		for (j = 0; j < (RTBUFSAMPS*NCHANS); j++)
		  outbuff[j] = 0.0;
      }


      if (!rtInteractive) {  // Ending condition
		if ((heapSize == 0) && (rtQSize == 0)) {
		  // printf("PLAYEM = 0\n");
		  playEm = 0;
		}
      }
    } // end playEm =========================================================
  
    if (out_port) {
      // Play zero'd buffers to avoid clicks
      if (play_audio) {
		int count = NCHANS * 2;
		sbuf = rtrescale(outbuff);
		for (j = 0; j < count; j++) 
		  rtsendsamps(sbuf);
      }  
      rtreportstats();   /* only if rtsetparams was called */
#ifdef LINUX
      close(out_port);
#endif
    }
#ifdef LINUX
    if (in_port) {
      close(in_port);
    }
#endif
    if (rtfileit) {
#ifdef USE_SNDLIB
      rtcloseout();
#else
      close(rtoutfile);
#endif //!USE_SNDLIB
    }

	cout << "\n";
	// cout << "EXITING inTraverse() FUNCTION *****\n";
	// exit(1);
  }

} /* extern "C" */
