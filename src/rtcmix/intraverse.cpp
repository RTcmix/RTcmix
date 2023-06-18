/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <RTcmix.h>
#include "prototypes.h"
#include <pthread.h>
#ifndef EMBEDDED
#include <iostream>
#endif
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "heap/heap.h"
#include "rtdefs.h"
#include <AudioDevice.h>
#include <Instrument.h>
#include <RTOption.h>
#include <bus.h>
#include "BusSlot.h"
#include "dbug.h"
#include <ugens.h>

#ifdef MULTI_THREAD
#include "TaskManager.h"
#include <vector>
#endif

#ifdef EMBEDDED
extern "C" {
	void checkForBang();		// DAS TODO: currently in main.cpp -- create header and move to new file
	void checkForVals();
	void checkForPrint();
	void notifyIsFinished(long long);
}
#endif

using namespace std;

#undef ALLBUG
#undef BBUG /* this one turns on bus debugging (verbose!) */
#undef DBUG
#undef WBUG	/* this new one turns on prints of where we are */
#undef IBUG	/* debug what Instruments are doing */

// Temporary globals

static FRAMETYPE bufEndSamp;
static int startupBufCount = 0;
static bool audioDone = true;   // set to false in runMainLoop

int RTcmix::runMainLoop(void)
{
	Bool audio_configured = NO;

    rtcmix_debug(NULL, "runMainLoop():  entering function");

	// Initialize everything ... cause it's good practice
	bufStartSamp = 0;  // current end sample for buffer
	bufEndSamp = bufsamps();
	startupBufCount = 0;
	audioDone = false;
	
	// This lets signal handler know that we have gotten to this point.
	audioLoopStarted = 1;

	// Wait for the ok to go ahead
	::pthread_mutex_lock(&audio_config_lock);
	if (!audio_config) {
		if (RTOption::print())
			RTPrintf("runMainLoop():  waiting for audio_config . . .\n");
	}
	::pthread_mutex_unlock(&audio_config_lock);

	while (!audio_configured) {
//        rtcmix_debug(NULL, "runMainLoop():  top of !audio_configured loop");
		::pthread_mutex_lock(&audio_config_lock);
		if (audio_config) {
			audio_configured = YES;
		}
		::pthread_mutex_unlock(&audio_config_lock);
#ifndef EMBEDDED
        // This interactive mode is specifically the standalone one - not the embedded one.
		if (interactive()) {
            int ret = 0;
			if (run_status == RT_GOOD || run_status == RT_PANIC)
				continue;
			else if (run_status == RT_SHUTDOWN)
				RTPrintf("runMainLoop:  shutting down\n");
            else if (run_status == RT_ERROR) {
				RTPrintf("runMainLoop:  shutting down due to error\n");
                ret = -1;
            }
			audioDone = true;
			return ret;
		}
#endif
       usleep(1000*100);   // no reason to run loop faster that 1 per 100 ms.
	}

#ifndef EMBEDDED
	if (audio_configured && interactive()) {
		if (RTOption::print())
			RTPrintf("runMainLoop():  audio configured.\n");
	}
#else
	rtcmix_debug(NULL, "runMainLoop():  audio configured.");
#endif

	// NOTE: audioin, aux and output buffers are zero'd during allocation

	if (rtsetparams_was_called()) {
		startupBufCount = 0;

		rtcmix_debug(NULL, "runMainLoop():  calling startAudio()");
		
#ifndef EMBEDDED
		if (RTcmix::bufTimeOffset > 0) {
            const FRAMETYPE bufOffset = (FRAMETYPE)(RTcmix::bufTimeOffset * sr());
			RTPrintf("Skipping %f seconds (%llu frames)", RTcmix::bufTimeOffset, (unsigned long long)bufOffset);
			run_status = RT_SKIP;
			int dot = 0, dotskip = (int)(sr()/bufsamps());	// dots in a second of audio
			while (bufStartSamp < bufOffset) {
                if (inTraverse(audioDevice, this) == false) {
                    audioDone = true;
                    return -1;    // Signal caller not to wait.
                }
				if (dot++ % dotskip == 0) {
					RTPrintf(".");
				}
			}
            RTPrintf("\n");
			run_status = RT_GOOD;
		}
#endif
		if (startAudio(inTraverse, doneTraverse, this) != 0) {
			audioDone = true;
			return -1;
		}
		rtcmix_debug(NULL, "runMainLoop():  exiting function");
		return 0;	// Playing, thru HW and/or to FILE.
	}
	audioDone = true;
	return -1;	// Not playing, signal caller not to wait.
}

int RTcmix::waitForMainLoop()
{
	rtcmix_debug(NULL, "waitForMainLoop():  entering function, audioDone = %d", audioDone);
    if (!audioDone) { RTPrintf("Playing...\n"); }
	while (!audioDone) {
		usleep(10000);
	}
	close();
	bufEndSamp = 0;		// reset
	rtcmix_debug(NULL, "waitForMainLoop():  exiting function");
	return 0;
}

// This is now the audio play callback
bool RTcmix::inTraverse(AudioDevice *device, void *arg)
{
//	RTcmix *RTCore = (RTcmix *) arg;
	Bool panic = NO;
	int bus = -1, bus_count = 0, busq = 0;
	int i;
	int bus_q_offset = 0;
    const int frameCount = bufsamps();

#ifdef WBUG
	RTPrintf("ENTERING inTraverse()\n");
#endif
#ifdef DBUG	  
	RTPrintf("\nEntering big loop ...........  bufStartSamp = %llu\n", (unsigned long long)bufStartSamp);
#endif

#ifdef EMBEDDED
	// BGG mm -- I need to find a better place to put this...
   bufEndSamp = bufStartSamp + frameCount;
#endif

	// send a buffer of zeros to audio output device.  NOTE:  This used to be
	// in the inTraverse init code.
	
	if (startupBufCount-- > 0) {
		return (rtsendzeros(device, false) == 0) ? true : false;
	}

	if (interactive() && run_status == RT_PANIC)
		panic = YES;
#ifdef EMBEDDED
	else if (interactive() && run_status == RT_FLUSH) {
		resetHeapAndQueue();
		rtsendzeros(device, false);
		run_status = RT_GOOD;
		notifyIsFinished(bufEndSamp);
		return true;
	}
#endif

	// Pop elements off rtHeap and insert into rtQueue +++++++++++++++++++++

	// deleteMin() returns top instrument if inst's start time is < bufEndSamp,
	// else NULL.  heapChunkStart is set in all cases
	
	FRAMETYPE heapChunkStart = 0;
	Instrument *Iptr;
    const BusSlot *iBus;

	while ((Iptr = rtHeap->deleteMin(bufEndSamp, &heapChunkStart)) != NULL) {
#ifdef DBUG
		RTPrintf("Iptr %p pulled from rtHeap\n", Iptr);
		RTPrintf("heapChunkStart = %lld\n", (long long)heapChunkStart);
#endif
		if (panic) {
#ifdef DBUG
			RTPrintf("Panic: Iptr %p unref'd\n", Iptr);
#endif
			Iptr->unref();
			continue;
		}

		// Because we know this instrument will be run during this slot,
		// perform final configuration on it if we are not interactive.
		// (If interactive, this is handled at init() time).

		if (!interactive()) {
#ifdef ALLBUG
			RTPrintf("Calling configure()\n");
#endif
			if (Iptr->configure(frameCount) != 0) {
#ifdef DBUG
				rtcmix_warn(NULL, "Inst configure error: Iptr %p unref'd", Iptr);
#endif
				Iptr->unref();
				continue;
			}
		}

        iBus = Iptr->getBusSlot();

		// DJT Now we push things onto different queues
		::pthread_mutex_lock(&bus_slot_lock);
		IBusClass bus_class = iBus->Class();
		switch (bus_class) {
		case TO_AUX:
			bus_count = iBus->auxout_count;
			bus_q_offset = 0;
			for(i=0;i<bus_count;i++) {
				bus = iBus->auxout[i];
				busq = bus+bus_q_offset;
#ifdef ALLBUG
				RTPrintf("Pushing on TO_AUX[%d] rtQueue\n", busq);
#endif
				rtQueue[busq].push(Iptr,heapChunkStart);
			}
			break;
		case AUX_TO_AUX:
			bus_count = iBus->auxout_count;
			bus_q_offset = busCount;
			for(i=0;i<bus_count;i++) {
				bus = iBus->auxout[i];
				busq = bus+bus_q_offset;
#ifdef ALLBUG
				RTPrintf("Pushing on AUX_TO_AUX[%d] rtQueue\n", busq);
#endif
				rtQueue[busq].push(Iptr,heapChunkStart);
			}
			break;
		case TO_OUT:
			bus_count = iBus->out_count;
			bus_q_offset = busCount*2;
			for(i=0;i<bus_count;i++) {
				bus = iBus->out[i];
				busq = bus+bus_q_offset;
#ifdef ALLBUG
				RTPrintf("Pushing on TO_OUT[%d] rtQueue\n", busq);
#endif
				rtQueue[busq].push(Iptr,heapChunkStart);
			}
			break;
		case TO_AUX_AND_OUT:
			bus_count = iBus->out_count;
			bus_q_offset = busCount;
			for(i=0;i<bus_count;i++) {
				bus = iBus->out[i];
				busq = bus+bus_q_offset;
#ifdef ALLBUG
				RTPrintf("Pushing on TO_OUT2[%d] rtQueue\n", busq);
#endif
				rtQueue[busq].push(Iptr,heapChunkStart);
			}
			bus_count = iBus->auxout_count;
			bus_q_offset = 2*busCount;
			for(i=0;i<bus_count;i++) {
				bus = iBus->auxout[i];
				busq = bus+bus_q_offset;
#ifdef ALLBUG
				RTPrintf("Pushing on TO_AUX2[%d] rtQueue\n", busq);
#endif
				rtQueue[busq].push(Iptr,heapChunkStart);
			}
			break;
		default:
			rterror("intraverse", "unknown bus_class");
			break;
		}
		::pthread_mutex_unlock(&bus_slot_lock);
	}
	// End rtHeap popping and rtQueue insertion ----------------------------

	BusType bus_type = BUS_OUT; // Default when none is set
	IBusClass qStatus = TO_AUX;
	short play_bus = 0;
	Bool aux_pb_done = NO;
	int rtQSize = 0, allQSize = 0;
    FRAMETYPE rtQchunkStart = 0;
#ifdef MULTI_THREAD
	vector<Instrument *>instruments;
	instruments.reserve(busCount);
#endif
	bool instrumentFound = false;
	// rtQueue[] playback shuffling ++++++++++++++++++++++++++++++++++++++++
	while (!aux_pb_done) {
		switch (qStatus) {
		case TO_AUX:
			bus_q_offset = 0;
			bus_type = BUS_AUX_OUT;
			::pthread_mutex_lock(&to_aux_lock);
#ifdef BBUG
                printf("TO_AUX: play_bus: %d\n", play_bus);
#endif
            assert(play_bus < busCount);
			bus = ToAuxPlayList[play_bus++];
			::pthread_mutex_unlock(&to_aux_lock);
			break;
		case AUX_TO_AUX:
			bus_q_offset = busCount;
			::pthread_mutex_lock(&aux_to_aux_lock);
#ifdef BBUG
                printf("AUX_TO_AUX: play_bus: %d\n", play_bus);
#endif
            assert(play_bus < busCount);
			bus = AuxToAuxPlayList[play_bus++];
			::pthread_mutex_unlock(&aux_to_aux_lock);
			bus_type = BUS_AUX_OUT;
			break;
		case TO_OUT:
			bus_q_offset = busCount*2;
			::pthread_mutex_lock(&to_out_lock);
#ifdef BBUG
                printf("TO_OUT: play_bus: %d\n", play_bus);
#endif
            assert(play_bus < busCount);
			bus = ToOutPlayList[play_bus++];
			::pthread_mutex_unlock(&to_out_lock);
			bus_type = BUS_OUT;
			break;
		default:
			rterror("intraverse", "unknown bus_class");
			break;
		}

#ifdef BBUG
        printf("after qStatus switch: bus = %d\n", bus);
#endif
		if (bus != -1) {
			busq = bus+bus_q_offset;
			rtQSize = rtQueue[busq].getSize();
			if (rtQSize > 0) {
				rtQchunkStart = rtQueue[busq].nextChunk();
				// DS ADDED
				assert(rtQchunkStart > 0 || bufStartSamp == 0);
			}
		}
		else {
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
#ifdef BBUG
				printf("aux_pb_done\n");
#endif
				aux_pb_done = YES;
				break;
			default:
				rterror("intraverse", "unknown bus_class");
				break;
			}
		}
#ifdef BBUG
		printf("bus: %d  busq: %d\n", bus, busq);
#endif
        
#ifdef MULTI_THREAD
		if (bus != -1) {
#if defined(BBUG)
			printf("\nAdding instruments for current slice [end = %.3f ms] and bus [%d]\n",
				   1000 * bufEndSamp/sr(), busq);
#endif
		}
		else {
#if defined(BBUG) || defined(DEBUG)
			printf("\nDone with bus type %d -- continuing\n", bus_type);
#endif
			continue;
		}
		// Play elements on queue (insert back in if needed) ++++++++++++++++++
		while (rtQSize > 0 && rtQchunkStart < bufEndSamp) {
			int chunksamps = 0;
			instrumentFound = true;
			
			Iptr = rtQueue[busq].pop(&rtQchunkStart);  // get next instrument off queue
#ifdef IBUG
            printf("Iptr %p popped from rtQueue %d at rtQchunkStart %lld\n", Iptr, busq, rtQchunkStart);
#endif			
			Iptr->set_ichunkstart(rtQchunkStart);

			FRAMETYPE endsamp = Iptr->getendsamp();

			// difference in sample start (countdown)
			int offset = int(rtQchunkStart - bufStartSamp);

			// DJT:  may have to expand here.  IE., conditional above
			// (rtQchunkStart >= bufStartSamp)
			// unlcear what that will do just now
			if (offset < 0) { // BGG: added this trap for robustness
#ifndef EMBEDDED
                printf("WARNING: the scheduler is behind the queue!\n");
                printf("bufStartSamp:  %ld\n", (long)bufStartSamp);
                printf("endsamp:  %ld\n", (long)endsamp);
#endif
				endsamp += offset;  // DJT:  added this (with hope)
				offset = 0;
			}

			Iptr->set_output_offset(offset);

			if (endsamp < bufEndSamp) {  // compute # of samples to write
				chunksamps = int(endsamp-rtQchunkStart);
			}
			else {
				chunksamps = int(bufEndSamp-rtQchunkStart);
			}
			if (chunksamps > frameCount) {
#ifndef EMBEDDED
                printf("ERROR: chunksamps is %ld - limiting to %ld\n", (long)chunksamps, (long)frameCount);
#endif
				chunksamps = frameCount;
			}
#ifdef DBUG
            printf("Begin playback iteration==========\n");
            printf("bufEndSamp:  %ld\n", (long)bufEndSamp);
            printf("frameCount:  %ld\n", (long)frameCount);
            printf("endsamp:  %ld\n", (long)endsamp);
            printf("offset:  %ld\n", (long)offset);
            printf("chunksamps:  %ld\n", (long)chunksamps);
#endif
			Iptr->setchunk(chunksamps);  // set "chunksamps"		 
						
			// DT_PANIC_MOD
			if (!panic) {
#ifdef IBUG
                printf("putting inst %p into taskmgr (bus_type %d) [%s]\n", Iptr, bus_type, Iptr->name());
#endif
				instruments.push_back(Iptr);
				taskManager->addTask<Instrument, int, BusType, int, &Instrument::exec>(Iptr, bus_type, bus);
			}
            else { // DT_PANIC_MOD ... just keep on incrementing endsamp
				endsamp += chunksamps;
            }
			rtQSize = rtQueue[busq].getSize();
		}	// while (rtQSize > 0 && rtQchunkStart < bufEndSamp)

		if (!instruments.empty()) {
#if defined(DBUG)
			printf("Done adding instruments for current slice\n");
			printf("waiting for %d instrument tasks...", (int) instruments.size());
#endif
			taskManager->waitForTasks(instruments);
        	RTcmix::mixToBus();
#if defined(DBUG)
            printf("done waiting\n");
#endif
#if defined(IBUG)
			printf("Re-queuing instruments\n");
#endif
		}
		for (vector<Instrument *>::iterator it = instruments.begin(); it != instruments.end(); ++it) {
			Iptr = *it;
			int chunksamps = Iptr->framesToRun();
			FRAMETYPE endsamp = Iptr->getendsamp();
			int inst_chunk_finished = Iptr->needsToRun();

			rtQchunkStart = Iptr->get_ichunkstart();    // We stored this value before placing into the vector
            
			// ReQueue or unref ++++++++++++++++++++++++++++++++++++++++++++++
			if (endsamp > bufEndSamp && !panic) {
#ifdef IBUG
                printf("re queueing inst %p on rtQueue %d\n", Iptr, busq);
#endif
				rtQueue[busq].push(Iptr,rtQchunkStart+chunksamps);   // put back onto queue
			}
			else {
				iBus = Iptr->getBusSlot();
				// unref only after all buses have played -- i.e., if inst_chunk_finished.
				// if not unref'd here, it means the inst still needs to run on another bus.
				if (qStatus == iBus->Class() && inst_chunk_finished) {
#ifdef IBUG
                    printf("unref'ing inst %p\n", Iptr);
#endif
					Iptr->unref();
					Iptr = NULL;
				}
			}  // end rtQueue or unref ----------------------------------------
			
			rtQSize = rtQueue[busq].getSize();
			if (rtQSize) {
				allQSize += rtQSize;
			}
#ifdef DBUG
            printf("rtQSize: %ld\n", (long)rtQSize);
            printf("rtQchunkStart:  %ld\n", (long)rtQchunkStart);
            printf("chunksamps:  %ld\n", (long)chunksamps);
            printf("Iteration done==========\n\n");
#endif
		} // end while() [Play elements on queue (insert back in if needed)] -----------
		instruments.clear();
	}  // end while (!aux_pb_done) --------------------------------------------------

#else   // MULTI_THREAD
    // Play elements on queue (insert back in if needed) ++++++++++++++++++
    while (rtQSize > 0 && rtQchunkStart < bufEndSamp && bus != -1) {
        int chunksamps = 0;
		instrumentFound = true;
        Iptr = rtQueue[busq].pop(&rtQchunkStart);  // get next instrument off queue
#ifdef IBUG
		printf("Iptr %p popped from rtQueue %d at rtQchunkStart %lld\n", Iptr, busq, rtQchunkStart);
#endif
        Iptr->set_ichunkstart(rtQchunkStart);
                
        FRAMETYPE endsamp = Iptr->getendsamp();
        
        // difference in sample start (countdown)
        int offset = int(rtQchunkStart - bufStartSamp);
        
        // DJT:  may have to expand here.  IE., conditional above
        // (rtQchunkStart >= bufStartSamp)
        // unlcear what that will do just now
        if (offset < 0) { // BGG: added this trap for robustness
#ifndef EMBEDDED
            printf("WARNING: the scheduler is behind the queue!\n");
            printf("bufStartSamp:  %ld\n", (long)bufStartSamp);
            printf("endsamp:  %ld\n", (long)endsamp);
#endif
            endsamp += offset;  // DJT:  added this (with hope)
            offset = 0;
        }
        
        Iptr->set_output_offset(offset);
        
        if (endsamp < bufEndSamp) {  // compute # of samples to write
            chunksamps = int(endsamp-rtQchunkStart);
        }
        else {
            chunksamps = int(bufEndSamp-rtQchunkStart);
        }
        if (chunksamps > frameCount) {
#ifndef EMBEDDED
            printf("ERROR: chunksamps is %ld - limiting to %ld\n", (long)chunksamps, (long)frameCount);
#endif
            chunksamps = frameCount;
        }
#ifdef DBUG
        printf("Begin playback iteration==========\n");
        printf("bufEndSamp:  %ld\n", (long)bufEndSamp);
        printf("frameCount:  %ld\n", (long)frameCount);
        printf("endsamp:  %ld\n", (long)endsamp);
        printf("offset:  %ld\n", (long)offset);
        printf("chunksamps:  %ld\n", (long)chunksamps);
#endif      
        Iptr->setchunk(chunksamps);  // set "chunksamps"		 
        
        int inst_chunk_finished = 0;
        
        // DT_PANIC_MOD
        if (!panic) {
#ifdef IBUG
            printf("Iptr->exec(%d, %d) [%s]\n", bus_type, bus, Iptr->name());
#endif
            inst_chunk_finished = Iptr->exec(bus_type, bus);    // write the samples * * * * * * * * * 
            endsamp = Iptr->getendsamp();
        }
        else // DT_PANIC_MOD ... just keep on incrementing endsamp
            endsamp += chunksamps;
        
        // ReQueue or unref ++++++++++++++++++++++++++++++++++++++++++++++
        if (endsamp > bufEndSamp && !panic) {
#ifdef IBUG
            printf("re queueing inst %p on rtQueue %d\n", Iptr, busq);
#endif
            rtQueue[busq].push(Iptr,rtQchunkStart+chunksamps);   // put back onto queue
        }
        else {
            iBus = Iptr->getBusSlot();
            // unref only after all buses have played -- i.e., if inst_chunk_finished.
            // if not unref'd here, it means the inst still needs to run on another bus.

            if (qStatus == iBus->Class() && inst_chunk_finished) {
#ifdef IBUG
                printf("unref'ing inst %p\n", Iptr);
#endif
                Iptr->unref();
				Iptr = NULL;
            }
        }  // end rtQueue or unref ----------------------------------------
        
        // DJT:  not sure this check before new rtQchunkStart is necessary
        rtQSize = rtQueue[busq].getSize();
        if (rtQSize) {
            rtQchunkStart = rtQueue[busq].nextChunk(); /* FIXME:  crapping out */
            allQSize += rtQSize;                /* in RT situation sometimes */
        }
#ifdef DBUG
        printf("rtQSize: %ld\n", (long)rtQSize);
        printf("rtQchunkStart:  %ld\n", (long)rtQchunkStart);
        printf("chunksamps:  %ld\n", (long)chunksamps);
        printf("Iteration done==========\n\n");
#endif
    } // end while() [Play elements on queue (insert back in if needed)] -----------
}  // end while (!aux_pb_done) --------------------------------------------------

#endif  // MULTI_THREAD

#ifdef EMBEDDED
	// Here is where we now call the "checkers" for Bang, Values, and Print	-- DAS
	checkForBang();
	checkForVals();
	checkForPrint();
#endif

	if (panic) {
		rtsendzeros(device, false);
	}
	else if (run_status != RT_SKIP) {
        // Write buf to audio device - - - - - - - - - - - - - - - - - - - - -
#ifdef DBUG
        printf("Writing samples----------\n");
        printf("bufEndSamp:  %ld\n", (long)bufEndSamp);
#endif

		if (rtsendsamps(device) != 0) {
#ifdef WBUG
			RTPrintf("EXITING inTraverse()\n");
#endif
			return false;
		}
	}

	elapsed += frameCount;
	bufStartSamp += frameCount;
	bufEndSamp += frameCount;

	// zero the buffers
	clear_aux_buffers();
	clear_output_buffers();

	// read in an input buffer (if audio input is active)
	if (rtrecord) {
		if (!panic && run_status != RT_SKIP)
			rtgetsamps(device);
	}

	bool playEm = true;
	
    // Check status from other threads
    if (run_status == RT_SHUTDOWN) {
#ifndef EMBEDDED
        RTPrintf("inTraverse:  shutting down\n");
#endif
        playEm = false;
    }
    else if (run_status == RT_ERROR) {
#ifndef EMBEDDED
        RTPrintf("inTraverse:  shutting down due to error\n");
#endif
        playEm = false;
    }

    const bool instrumentQueueIsEmpty = rtHeap->getSize() == 0 && allQSize == 0;

	if (!interactive()) {  // Ending condition
		if (instrumentQueueIsEmpty) {
#ifdef ALLBUG
			printf("heapSize:  %ld\n", (long)rtHeap->getSize());
			printf("rtQSize:  %ld\n", (long)rtQSize);
			printf("PLAYEM = FALSE\n");
			printf("The end\n\n");
#endif
			playEm = false;
#ifdef EMBEDDED
            if (instrumentFound) {
                notifyIsFinished(bufEndSamp);
            }
#endif
		}
	}
	else {
#ifdef EMBEDDED
		if (instrumentFound && instrumentQueueIsEmpty) {
			notifyIsFinished(bufEndSamp);
		}
#endif
		if (panic && run_status == RT_GOOD) {
#ifndef EMBEDDED
            RTPrintf("inTraverse:  panic mode finished\n");
#endif
			panic = NO;
		}
		// DT_PANIC_MOD
		if (panic && instrumentQueueIsEmpty)
			run_status = RT_GOOD;
	}
#ifdef WBUG
    RTPrintf("EXITING inTraverse()\n");
#endif
	return playEm;
}

void  RTcmix::resetHeapAndQueue()
{
	delete rtHeap;
	delete [] rtQueue;
	rtHeap = NULL;
	rtQueue = NULL;
	
	rtHeap = new heap;
	rtQueue = new RTQueue[busCount*3];
}

bool RTcmix::doneTraverse(AudioDevice *device, void *arg)
{
#ifdef WBUG
	RTPrintf("ENTERING doneTraverse()\n");
#endif
    callStopCallbacks();
#ifndef EMBEDDED
	if (RTOption::print())
		RTPrintf("\nclosing...\n");
	RTPrintf("Output duration: %.2f seconds\n", bufEndSamp / sr());
	rtreportstats(device);
	if (RTOption::print())
		RTPrintf("\n");
#endif
	audioDone = true;	// This signals waitForMainLoop()

#ifdef WBUG
	RTPrintf("EXITING doneTraverse()\n");
#endif

	return true;
}

