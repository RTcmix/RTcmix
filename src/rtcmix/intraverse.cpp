#include <pthread.h>
#include <iostream.h>
#include "../rtstuff/heap/heap.h"
#include "../rtstuff/rt.h"
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
   int rtInteractive;
}

int noaudio;  // to delay socket parsing

extern int out_port;  // Audio file descriptor
extern int in_port;  // Audio file descriptor

extern heap rtHeap;  // main heap structure from main.C
extern rtQueue rtQueue;  // real time queue

extern pthread_mutex_t heapLock;

double baseTime;
long elapsed;
extern int audio_config;  // Flag to wait for

extern int rtfileit,rtoutfile,rtoutswap,play_audio;

extern "C" int audio_on;

/* traverse.C and sys/rtwritesamps.C import this */
unsigned long bufStartSamp;

extern float *outbuff, *outbptr; /* defined in main.C */
extern short *inbuff;


extern "C" {
  void *inTraverse()
  {
    short rtInst;
    short playEm;
    int j,chunksamps;
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

    while(playEm) {

      pthread_mutex_lock(&heapLock);
      heapSize = rtHeap.getSize();
      if (heapSize > 0) {
	heapChunkStart = rtHeap.getTop();
      }
      pthread_mutex_unlock(&heapLock);

      // Put elements onto rtQueue
      while ((heapChunkStart < bufEndSamp) && (heapSize > 0)) {
        rtInst = 1;
        pthread_mutex_lock(&heapLock);
	Iptr = rtHeap.deleteMin();  // get next instrument off heap
	pthread_mutex_unlock(&heapLock);
	if (!Iptr)
	  break;

	Iptr->setchunkstart(heapChunkStart);
	rtQueue.push(Iptr);

	pthread_mutex_lock(&heapLock);
	heapSize = rtHeap.getSize();
	if (heapSize > 0)
	  heapChunkStart = rtHeap.getTop();
        pthread_mutex_unlock(&heapLock);
      }

      rtQSize = rtQueue.getSize();
      // cout << "rtQSize:  " << rtQSize << endl;
    
      // Play elements on queue
      if (rtQSize > 0) {
      
	chunkStart = rtQueue.nextChunk();
#ifdef ALLBUG
	cout << "Q-chunkStart:  " << chunkStart << endl;
	cout << "bufEndSamp:  " << bufEndSamp << endl;
	cout << "RTBUFSAMPS:  " << RTBUFSAMPS << endl;
#endif      
	// get next Instrument off queue
	if (chunkStart < bufEndSamp) {
	  // cout << "Popping instrument off queue\n";
	  Iptr = rtQueue.pop();  // get next instrument off heap
	
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
	  // cout << "Executing instrument\n";
	  // read in an input buffer (if audio input is active)
	  // DT:  this IS where we should be doing this ?
	  // if (audio_on) {
	  //   cout << "Reading from audio port\n";
	  //   rtgetsamps();
	  // }

	  Iptr->run();    // write the samples
	
	  if (endsamp > bufEndSamp) {
	    Iptr->setchunkstart(chunkStart+chunksamps);  // reset chunkStart
#ifdef ALLBUG
	    cout << "inTraverse():  re queueing instrument\n";
#endif
	    rtQueue.push(Iptr);   // put back onto queue
	  }
	  else {
	    delete Iptr;
	  }
	}
      
	// we need to write a new buffer
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
	  if (audio_on) 
	    // cout << "Reading data from audio port\n";
	    rtgetsamps();
	}
      
      }
      else {
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
    }
  
    if (out_port) {
      // Play zero'd buffers to avoid clicks
      if (play_audio) {
	int count = NCHANS * 2;
	sbuf = rtrescale(outbuff);
	for (j = 0; j < count; j++) 
	  rtsendsamps(sbuf);
      }  
      rtreportstats();   /* only if rtsetparams was called */
      close(out_port);
    }
    if (in_port) {
      close(in_port);
    }
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
