/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

// BGGx ww -- for Sleep()
#include <windows.h>

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
#include <Option.h>
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
}
#endif

using namespace std;

#undef TBUG
#undef ALLBUG
#undef DBUG
#undef WBUG	/* this new one turns on prints of where we are */
#undef IBUG	/* debug what Instruments are doing */

// Temporary globals

static FRAMETYPE bufEndSamp;
static int startupBufCount = 0;
static bool audioDone = false;

int RTcmix::runMainLoop(void)
{
	Bool audio_configured = NO;

#ifdef WBUG	
	RTPrintf("ENTERING runMainLoop() FUNCTION *****\n");
#endif

	// Initialize everything ... cause it's good practice
	bufStartSamp = 0;  // current end sample for buffer
	bufEndSamp = RTBUFSAMPS;
	startupBufCount = 0;
	audioDone = false;
	
	// This lets signal handler know that we have gotten to this point.
	audioLoopStarted = 1;

	// Wait for the ok to go ahead
	::pthread_mutex_lock(&audio_config_lock);
	if (!audio_config) {
		if (Option::print())
			RTPrintf("runMainLoop():  waiting for audio_config . . .\n");
	}
	::pthread_mutex_unlock(&audio_config_lock);

	while (!audio_configured) {
		::pthread_mutex_lock(&audio_config_lock);
		if (audio_config) {
			audio_configured = YES;
		}
		::pthread_mutex_unlock(&audio_config_lock);
		if (rtInteractive) {
			if (run_status == RT_GOOD || run_status == RT_PANIC)
				continue;
			else if (run_status == RT_SHUTDOWN)
				RTPrintf("runMainLoop:  shutting down\n");
			else if (run_status == RT_ERROR)
				RTPrintf("runMainLoop:  shutting down due to error\n");
			audioDone = true;
			return -1;
		}
	}

#ifndef EMBEDDED
	if (audio_configured && rtInteractive) {
		if (Option::print())
			RTPrintf("runMainLoop():  audio set.\n");
	}
#else
	rtcmix_debug(NULL, "runMainLoop():  audio set.");
	rtInteractive = 1;
#endif

	// NOTE: audioin, aux and output buffers are zero'd during allocation

	if (rtsetparams_was_called()) {
		startupBufCount = 0;

		rtcmix_debug(NULL, "runMainLoop():  calling startAudio()");
		
		if (bufOffset > 0) {
			RTPrintf("Skipping %llu frames", (unsigned long long)bufOffset);
			run_status = RT_SKIP;
			int dot = 0, dotskip = (int)(sr()/bufsamps());	// dots in a second of audio
			while (bufStartSamp < bufOffset) {
				inTraverse(audioDevice, this);
				if (dot++ % dotskip == 0) {
					RTPrintf(".");
				}
			}
			RTPrintf("\nPlaying.\n");
			run_status = RT_GOOD;
		}

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
	rtcmix_debug(NULL, "waitForMainLoop():  entering function");
	while (!audioDone) {
		// BGGx ww
		//usleep(10000);
		Sleep(10);
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
	short bus = -1, bus_count = 0, busq = 0;
	int i;
	short bus_q_offset = 0;

#ifdef WBUG
	RTPrintf("ENTERING inTraverse()\n");
#endif
#ifdef DBUG	  
	RTPrintf("\nEntering big loop ...........  bufStartSamp = %llu\n", (unsigned long long)bufStartSamp);
#endif

#ifdef EMBEDDED
	// BGG mm -- I need to find a better place to put this...
   bufEndSamp = bufStartSamp + RTBUFSAMPS;
#endif

	// send a buffer of zeros to audio output device.  NOTE:  This used to be
	// in the inTraverse init code.
	
	if (startupBufCount-- > 0) {
		return (rtsendzeros(device, false) == 0) ? true : false;
	}

	if (rtInteractive && run_status == RT_PANIC)
		panic = YES;
#ifdef EMBEDDED
	else if (rtInteractive && run_status == RT_FLUSH) {
		resetHeapAndQueue();
		rtsendzeros(device, false);
		run_status = RT_GOOD;
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

		if (!rtInteractive) {
#ifdef ALLBUG
			RTPrintf("Calling configure()\n");
#endif
			if (Iptr->configure(RTBUFSAMPS) != 0) {
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
	// rtQueue[] playback shuffling ++++++++++++++++++++++++++++++++++++++++
	while (!aux_pb_done) {
		switch (qStatus) {
		case TO_AUX:
			bus_q_offset = 0;
			bus_type = BUS_AUX_OUT;
			::pthread_mutex_lock(&to_aux_lock);
			bus = ToAuxPlayList[play_bus++];
			::pthread_mutex_unlock(&to_aux_lock);
			break;
		case AUX_TO_AUX:
			bus_q_offset = busCount;
			::pthread_mutex_lock(&aux_to_aux_lock);
			bus = AuxToAuxPlayList[play_bus++];
			::pthread_mutex_unlock(&aux_to_aux_lock);
			bus_type = BUS_AUX_OUT;
			break;
		case TO_OUT:
			bus_q_offset = busCount*2;
			::pthread_mutex_lock(&to_out_lock);
			bus = ToOutPlayList[play_bus++];
			::pthread_mutex_unlock(&to_out_lock);
			bus_type = BUS_OUT;
			break;
		default:
			rterror("intraverse", "unknown bus_class");
			break;
		}

		if (bus != -1) {
			busq = bus+bus_q_offset;
			rtQSize = rtQueue[busq].getSize();
			if (rtQSize > 0) {
				rtQueue[busq].sort();
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
#ifdef ALLBUG
				printf("aux_pb_done\n");
#endif
				aux_pb_done = YES;
				break;
			default:
				rterror("intraverse", "unknown bus_class");
				break;
			}
		}
#ifdef ALLBUG
		printf("bus: %d  busq: %d\n", bus, busq);
#endif
        
#ifdef MULTI_THREAD
		if (bus != -1) {
#if defined(IBUG)
			printf("\nAdding instruments for current slice [end = %.3f ms] and bus [%d]\n",
				   1000 * bufEndSamp/SR, busq);
#endif
		}
		else {
#if defined(IBUG) || defined(DEBUG)
			printf("\nDone with bus type %d -- continuing\n", bus_type);
#endif
			continue;
		}
		// Play elements on queue (insert back in if needed) ++++++++++++++++++
		while (rtQSize > 0 && rtQchunkStart < bufEndSamp) {
			int chunksamps = 0;
			            
			Iptr = rtQueue[busq].pop(&rtQchunkStart);  // get next instrument off queue
#ifdef DBUG
			cout << "Iptr " << (void *) Iptr << " popped from rtQueue " << busq << " at rtQchunkStart " << rtQchunkStart << endl;
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
				cout << "WARNING: the scheduler is behind the queue!" << endl;
				cout << "bufStartSamp:  " << bufStartSamp << endl;
				cout << "endsamp:  " << endsamp << endl;
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
			if (chunksamps > RTBUFSAMPS) {
#ifndef EMBEDDED
				cout << "ERROR: chunksamps: " << chunksamps << " limiting to " << RTBUFSAMPS << endl;
#endif
				chunksamps = RTBUFSAMPS;
			}
#ifdef DBUG
			cout << "Begin playback iteration==========\n";
			cout << "bufEndSamp:  " << bufEndSamp << endl;
			cout << "RTBUFSAMPS:  " << RTBUFSAMPS << endl;
			cout << "endsamp:  " << endsamp << endl;
			cout << "offset:  " << offset << endl;
			cout << "chunksamps:  " << chunksamps << endl;
#endif      
			Iptr->setchunk(chunksamps);  // set "chunksamps"		 
						
			// DT_PANIC_MOD
			if (!panic) {
#ifdef IBUG
				cout << "putting inst " << (void *) Iptr << " into taskmgr (bustype " << bus_type << ") [" << Iptr->name() << "]\n";
#endif
				instruments.push_back(Iptr);
				taskManager->addTask<Instrument, int, BusType, int, &Instrument::exec>(Iptr, bus_type, bus);
			}
			else // DT_PANIC_MOD ... just keep on incrementing endsamp
				endsamp += chunksamps;
			rtQSize = rtQueue[busq].getSize();
		}	// while (rtQSize > 0 && rtQchunkStart < bufEndSamp)

		if (!instruments.empty()) {
#if defined(DBUG) || defined(IBUG)
			cout << "Done adding instruments for current slice\n";
			cout << "waiting for " << instruments.size() << " instrument tasks..." << endl;
#endif
			taskManager->waitForTasks(instruments);
        	RTcmix::mixToBus();
#if defined(DBUG) || defined(IBUG)
			cout << "done waiting" << endl;
#endif
#if defined(IBUG)
			cout << "Re-queuing instruments\n";
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
#ifdef DBUG
				cout << "re queueing inst " << (void *) Iptr << " on rtQueue " << busq << endl;
#endif
				rtQueue[busq].push(Iptr,rtQchunkStart+chunksamps);   // put back onto queue
			}
			else {
				iBus = Iptr->getBusSlot();
				// unref only after all buses have played -- i.e., if inst_chunk_finished.
				// if not unref'd here, it means the inst still needs to run on another bus.
				if (qStatus == iBus->Class() && inst_chunk_finished) {
#ifdef DBUG
					cout << "unref'ing inst " << (void *) Iptr << endl;
#endif
					Iptr->unref();
				}
			}  // end rtQueue or unref ----------------------------------------
			
			rtQSize = rtQueue[busq].getSize();
			if (rtQSize) {
				allQSize += rtQSize;
			}
#ifdef DBUG
			cout << "rtQSize: " << rtQSize << endl;
			cout << "rtQchunkStart:  " << rtQchunkStart << endl;
			cout << "chunksamps:  " << chunksamps << endl;
			cout << "Iteration done==========\n";
#endif
		} // end while() [Play elements on queue (insert back in if needed)] -----------
		instruments.clear();
	}  // end while (!aux_pb_done) --------------------------------------------------

#else   // MULTI_THREAD
    
    // Play elements on queue (insert back in if needed) ++++++++++++++++++
    while (rtQSize > 0 && rtQchunkStart < bufEndSamp && bus != -1) {
        int chunksamps = 0;
                
        Iptr = rtQueue[busq].pop(&rtQchunkStart);  // get next instrument off queue
#ifdef DBUG
		cout << "Iptr " << (void *) Iptr << " popped from rtQueue " << busq << " at rtQchunkStart " << rtQchunkStart << endl;
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
            cout << "WARNING: the scheduler is behind the queue!" << endl;
            cout << "bufStartSamp:  " << bufStartSamp << endl;
            cout << "endsamp:  " << endsamp << endl;
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
        if (chunksamps > RTBUFSAMPS) {
#ifndef EMBEDDED
            cout << "ERROR: chunksamps: " << chunksamps << " limiting to " << RTBUFSAMPS << endl;
#endif
            chunksamps = RTBUFSAMPS;
        }
#ifdef DBUG
        cout << "Begin playback iteration==========\n";
        cout << "bufEndSamp:  " << bufEndSamp << endl;
        cout << "RTBUFSAMPS:  " << RTBUFSAMPS << endl;
        cout << "endsamp:  " << endsamp << endl;
        cout << "offset:  " << offset << endl;
        cout << "chunksamps:  " << chunksamps << endl;
#endif      
        Iptr->setchunk(chunksamps);  // set "chunksamps"		 
        
        int inst_chunk_finished = 0;
        
        // DT_PANIC_MOD
        if (!panic) {
#ifdef TBUG
            cout << "Iptr->exec(" << bus_type << "," << bus << ") [" << Iptr->name() << "]\n";
#endif
            inst_chunk_finished = Iptr->exec(bus_type, bus);    // write the samples * * * * * * * * * 
            endsamp = Iptr->getendsamp();
        }
        else // DT_PANIC_MOD ... just keep on incrementing endsamp
            endsamp += chunksamps;
        
        // ReQueue or unref ++++++++++++++++++++++++++++++++++++++++++++++
        if (endsamp > bufEndSamp && !panic) {
#ifdef DBUG
            cout << "re queueing inst " << (void *) Iptr << " on rtQueue " << busq << endl;
#endif
            rtQueue[busq].push(Iptr,rtQchunkStart+chunksamps);   // put back onto queue
        }
        else {
            iBus = Iptr->getBusSlot();
            // unref only after all buses have played -- i.e., if inst_chunk_finished.
            // if not unref'd here, it means the inst still needs to run on another bus.

            if (qStatus == iBus->Class() && inst_chunk_finished) {
#ifdef DBUG
                cout << "unref'ing inst " << (void *) Iptr << endl;
#endif
                Iptr->unref();
            }
        }  // end rtQueue or unref ----------------------------------------
        
        // DJT:  not sure this check before new rtQchunkStart is necessary
        rtQSize = rtQueue[busq].getSize();
        if (rtQSize) {
            rtQueue[busq].sort();
            rtQchunkStart = rtQueue[busq].nextChunk(); /* FIXME:  crapping out */
            allQSize += rtQSize;                /* in RT situation sometimes */
        }
#ifdef DBUG
        cout << "rtQSize: " << rtQSize << endl;
        cout << "rtQchunkStart:  " << rtQchunkStart << endl;
        cout << "chunksamps:  " << chunksamps << endl;
        cout << "Iteration done==========\n";
#endif
    } // end while() [Play elements on queue (insert back in if needed)] -----------
}  // end while (!aux_pb_done) --------------------------------------------------

#endif  // MULTI_THREAD

	// Write buf to audio device - - - - - - - - - - - - - - - - - - - - -
#ifdef DBUG
	cout << "Writing samples----------\n";
	cout << "bufEndSamp:  " << bufEndSamp << endl;
#endif

#ifdef EMBEDDED
	// Here is where we now call the "checkers" for Bang, Values, and Print	-- DAS
// BGGx -- use the 'polling' approach in Unity
//	checkForBang();
//	checkForVals();
//	checkForPrint();
#endif

	if (panic) {
		rtsendzeros(device, false);
	}
	else if (run_status != RT_SKIP) {
		if (rtsendsamps(device) != 0) {
#ifdef WBUG
			cout << "EXITING inTraverse()\n";
#endif
			return false;
		}
	}

	elapsed += RTBUFSAMPS;	
	bufStartSamp += RTBUFSAMPS;
	bufEndSamp += RTBUFSAMPS;

	// zero the buffers
	clear_aux_buffers();
	clear_output_buffers();

	// read in an input buffer (if audio input is active)
	if (rtrecord) {
		// cout << "Reading data from audio device\n";
		if (!panic && run_status != RT_SKIP)
			rtgetsamps(device);
	}

	bool playEm = true;
	
	if (!rtInteractive) {  // Ending condition
		if ((rtHeap->getSize() == 0) && (allQSize == 0)) {
#ifdef ALLBUG
			cout << "heapSize:  " << rtHeap->getSize() << endl;
			cout << "rtQSize:  " << rtQSize << endl;
			cout << "PLAYEM = FALSE\n";
			cout << "The end\n\n";
#endif
			playEm = false;
		}
	}
	else {
		// Check status from other threads
		if (run_status == RT_SHUTDOWN) {
#ifndef EMBEDDED
			cout << "inTraverse:  shutting down" << endl;
#endif
			playEm = false;
		}
		else if (run_status == RT_ERROR) {
#ifndef EMBEDDED
			cout << "inTraverse:  shutting down due to error" << endl;
#endif
			playEm = false;
		}
		else if (panic && run_status == RT_GOOD) {
#ifndef EMBEDDED
			cout << "inTraverse:  panic mode finished" << endl;
#endif
			panic = NO;
		}
		// DT_PANIC_MOD
		if (panic && (rtHeap->getSize() == 0) && (allQSize == 0))
			run_status = RT_GOOD;
	}
#ifdef WBUG
cout << "EXITING inTraverse()\n";
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
	cout << "ENTERING doneTraverse()\n";
#endif
#ifndef EMBEDDED
	if (Option::print())
		cout << "\nclosing...\n";
	printf("Output duration: %.2f seconds\n", bufEndSamp / sr());
	rtreportstats(device);
	if (Option::print())
		cout << "\n";
#endif
	audioDone = true;	// This signals waitForMainLoop()

#ifdef WBUG
	cout << "EXITING doneTraverse() FUNCTION *****\n";
#endif

	return true;
}

