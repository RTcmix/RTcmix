/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/
#include <globals.h>
#include <prototypes.h>
#include <pthread.h>
#include <iostream.h>
#include <stdio.h>
#include <unistd.h>
//#include <sys/time.h>           // DT: 3/97 needed for time function
#include <assert.h>
#include "../H/audio_port.h"    // JGG: for ZERO_FRAMES_BEFORE
#include "../rtstuff/heap/heap.h"
#include "../rtstuff/rtdefs.h"
#include <Instrument.h>
#include <bus.h>
#include "../H/dbug.h"

//#define TBUG
//#define ALLBUG
//#define DBUG

// DT:  main heap structure used to queue instruments
// D.S. Taken out of globals.h since only defined here.

heap rtHeap;  
 
extern "C" void *inTraverse(void *arg);

void *inTraverse(void *arg)
{
	short playEm = 0;
	int i,chunksamps;
	int rtQSize = 0, allQSize;
	int offset;
	int keepGoing;
	int dummy;
	short bus_q_offset;

	Instrument *Iptr;
	const BusSlot *iBus;

	unsigned long bufEndSamp, endsamp;
	unsigned long rtQchunkStart = 0;
	unsigned long heapChunkStart = 0;

	//   struct timeval tv;
	//   struct timezone tz;
	//   double sec,usec;

	Bool aux_pb_done,frame_done, panic = NO;
	Bool audio_configured = NO;
	short bus,bus_count,play_bus,busq = 0;
	IBusClass bus_class,qStatus;
	BusType bus_type = BUS_OUT; // Default when none is set

#ifdef ALLBUG	
	cout << "ENTERING inTraverse() FUNCTION *****\n";
#endif

	// Wait for the ok to go ahead
	pthread_mutex_lock(&audio_config_lock);
	if (!audio_config) {
		if (print_is_on)
			cout << "inTraverse():  waiting for audio_config . . .\n";
	}
	pthread_mutex_unlock(&audio_config_lock);

	while (!audio_configured) {
		pthread_mutex_lock(&audio_config_lock);
		if (audio_config) {
			audio_configured = YES;
		}
		pthread_mutex_unlock(&audio_config_lock);
		if (rtInteractive)
			{
				if (run_status == RT_GOOD || run_status == RT_PANIC)
					continue;
				else if (run_status == RT_SHUTDOWN)
					cout << "inTraverse():  shutting down" << endl;
				else if (run_status == RT_ERROR)
					cout << "inTraverse():  shutting down due to error" << endl;
				return NULL;
			}
	}

	if (audio_configured && rtInteractive) {
		if (print_is_on)
			cout << "inTraverse():  audio set." << endl;
	}

	// Initialize everything ... cause it's good practice
	bufStartSamp = 0;  // current end sample for buffer
	bufEndSamp = RTBUFSAMPS;
	bus_q_offset = 0;
	bus = -1;  // Don't play

	// NOTE: audioin, aux and output buffers are zero'd during allocation

	if (rtsetparams_called) {
		for (i = 0; i < ZERO_FRAMES_BEFORE / RTBUFSAMPS; i++)
			rtsendzeros(0);  // send a buffer of zeros to audio output device

		playEm = 1;
	}

	// read in an input buffer (if audio input is active)
	if (audio_on) {
		rtgetsamps();
		rtsendzeros(0);  // send a buffer of zeros to audio device
	}

	// Try and be tight with time
	//   gettimeofday(&tv, &tz);
	//   sec = (double)tv.tv_sec;
	//   usec = (double)tv.tv_usec;
	//   pthread_mutex_lock(&schedtime_lock);
	//   baseTime = (sec * 1e6) + usec;
	//   pthread_mutex_unlock(&schedtime_lock);

	while(playEm) { // the big loop ++++++++++++++++++++++++++++++++++++++++++

#ifdef DBUG	  
		printf("Entering big loop .....................\n");
#endif

		if (rtInteractive && run_status == RT_PANIC)
			panic = YES;

		// Pop elements off rtHeap and insert into rtQueue +++++++++++++++++++++
	
		// deleteMin() returns top instrument if inst's start time is < bufEndSamp,
		// else NULL.  heapChunkStart is set in all cases
	
		while ((Iptr = rtHeap.deleteMin(bufEndSamp, &heapChunkStart)) != NULL) 
			{

#ifdef DBUG
				cout << "Iptr " << (void *) Iptr << " pulled from rtHeap" << endl;
				cout << "heapChunkStart = " << heapChunkStart << endl;
#endif

				if (panic) {
#ifdef DBUG
					cout << "Panic: Iptr " << (void *) Iptr << " unref'd" << endl;
#endif
					Iptr->unref();
					continue;
				}
	  
				// Because we know this instrument will be run during this slot,
				// perform final configuration on it if we are not interactive.
				// (If interactive, this is handled at init() time).

				if (!rtInteractive) {
#ifdef ALLBUG
					cout << "Calling configure()" << endl;
#endif
					Iptr->configure();
				}
	  
				iBus = Iptr->getBusSlot();

				// DJT Now we push things onto different queues
				pthread_mutex_lock(&bus_slot_lock);
				bus_class = iBus->Class();
				switch (bus_class) {
				case TO_AUX:
					bus_count = iBus->auxout_count;
					bus_q_offset = 0;
					for(i=0;i<bus_count;i++) {
						bus = iBus->auxout[i];
						busq = bus+bus_q_offset;
#ifdef ALLBUG
						cout << "Pushing on TO_AUX[" << busq << "] rtQueue\n";
#endif
						rtQueue[busq].push(Iptr,heapChunkStart);
					}
					break;
				case AUX_TO_AUX:
					bus_count = iBus->auxout_count;
					bus_q_offset = MAXBUS;
					for(i=0;i<bus_count;i++) {
						bus = iBus->auxout[i];
						busq = bus+bus_q_offset;
#ifdef ALLBUG
						cout << "Pushing on AUX_TO_AUX[" << busq << "] rtQueue\n";
#endif
						rtQueue[busq].push(Iptr,heapChunkStart);
					}
					break;
				case TO_OUT:
					bus_count = iBus->out_count;
					bus_q_offset = MAXBUS*2;
					for(i=0;i<bus_count;i++) {
						bus = iBus->out[i];
						busq = bus+bus_q_offset;
#ifdef ALLBUG
						cout << "Pushing on TO_OUT[" << busq << "] rtQueue\n";
#endif
						rtQueue[busq].push(Iptr,heapChunkStart);
					}
					break;
				case TO_AUX_AND_OUT:
					bus_count = iBus->out_count;
					bus_q_offset = MAXBUS;
					for(i=0;i<bus_count;i++) {
						bus = iBus->out[i];
						busq = bus+bus_q_offset;
#ifdef ALLBUG
						cout << "Pushing on TO_OUT2[" << busq << "] rtQueue\n";
#endif
						rtQueue[busq].push(Iptr,heapChunkStart);
					}
					bus_count = iBus->auxout_count;
					bus_q_offset = 2*MAXBUS;
					for(i=0;i<bus_count;i++) {
						bus = iBus->auxout[i];
						busq = bus+bus_q_offset;
#ifdef ALLBUG
						cout << "Pushing on TO_AUX2[" << busq << "] rtQueue\n";
#endif
						rtQueue[busq].push(Iptr,heapChunkStart);
					}
					break;
				default:
					cout << "ERROR (intraverse): unknown bus_class\n";
					break;
				}
				pthread_mutex_unlock(&bus_slot_lock);
			}
		// End rtHeap popping and rtQueue insertion ----------------------------

		qStatus = TO_AUX;
		play_bus = 0;
		aux_pb_done = NO;
		allQSize = 0;

		// rtQueue[] playback shuffling ++++++++++++++++++++++++++++++++++++++++
		while (!aux_pb_done) {

			switch (qStatus) {
			case TO_AUX:
				bus_q_offset = 0;
				bus_type = BUS_AUX_OUT;
				pthread_mutex_lock(&to_aux_lock);
				bus = ToAuxPlayList[play_bus++];
				pthread_mutex_unlock(&to_aux_lock);
				break;
			case AUX_TO_AUX:
				bus_q_offset = MAXBUS;
				pthread_mutex_lock(&aux_to_aux_lock);
				bus = AuxToAuxPlayList[play_bus++];
				pthread_mutex_unlock(&aux_to_aux_lock);
				bus_type = BUS_AUX_OUT;
				break;
			case TO_OUT:
				bus_q_offset = MAXBUS*2;
				pthread_mutex_lock(&to_out_lock);
				bus = ToOutPlayList[play_bus++];
				pthread_mutex_unlock(&to_out_lock);
				bus_type = BUS_OUT;
				break;
			default:
				cout << "ERROR (intraverse): unknown bus_class\n";
				break;
			}

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
#ifdef ALLBUG
					cout << "aux_pb_done\n";
#endif
					aux_pb_done = YES;
					break;
				default:
					cout << "ERROR (intraverse): unknown bus_class\n";
					break;
				}
			}
#ifdef ALLBUG
			cout << "bus: " << bus << endl;
			cout << "busq:  " << busq << endl;
#endif

			// Play elements on queue (insert back in if needed) ++++++++++++++++++
			while (rtQSize > 0 && rtQchunkStart < bufEndSamp && bus != -1) {
				Iptr = rtQueue[busq].pop();  // get next instrument off queue
#ifdef DBUG
				cout << "Iptr " << (void *) Iptr << " popped from rtQueue " << busq << endl;
#endif			
				iBus = Iptr->getBusSlot();
				Iptr->set_ichunkstart(rtQchunkStart);

				endsamp = Iptr->getendsamp();

				// difference in sample start (countdown)
				offset = rtQchunkStart - bufStartSamp;  

				// DJT:  may have to expand here.  IE., conditional above
				// (rtQchunkStart >= bufStartSamp)
				// unlcear what that will do just now
				if (offset < 0) { // BGG: added this trap for robustness
					cout << "WARNING: the scheduler is behind the queue!" << endl;
					cout << "rtQchunkStart:  " << rtQchunkStart << endl;
					cout << "bufStartSamp:  " << bufStartSamp << endl;
					cout << "endsamp:  " << endsamp << endl;
					endsamp += offset;  // DJT:  added this (with hope)
					offset = 0;
				}

				Iptr->set_output_offset(offset);

				if (endsamp < bufEndSamp) {  // compute # of samples to write
					chunksamps = endsamp-rtQchunkStart;
				}
				else {
					chunksamps = bufEndSamp-rtQchunkStart;
				}
				if (chunksamps > RTBUFSAMPS)
					{
						cout << "ERROR: chunksamps: " << chunksamps << " limiting to " << RTBUFSAMPS << endl;
						chunksamps = RTBUFSAMPS;
					}

#ifdef DBUG
				cout << "Begin playback iteration==========\n";
				cout << "Q-rtQchunkStart:  " << rtQchunkStart << endl;
				cout << "bufEndSamp:  " << bufEndSamp << endl;
				cout << "RTBUFSAMPS:  " << RTBUFSAMPS << endl;
				cout << "endsamp:  " << endsamp << endl;
				cout << "offset:  " << offset << endl;
				cout << "chunksamps:  " << chunksamps << endl;
#endif      

				Iptr->setchunk(chunksamps);  // set "chunksamps"		 
#ifdef TBUG
				cout << "Iptr->exec(" << bus_type << "," << bus << ")\n";
#endif		  
				// DT_PANIC_MOD
				if (!panic) {
					Iptr->exec(bus_type, bus);    // write the samples * * * * * * * * * 
					endsamp = Iptr->getendsamp();
				}
				else // DT_PANIC_MOD ... just keep on incrementing endsamp
					endsamp += chunksamps;

				// ReQueue or unref ++++++++++++++++++++++++++++++++++++++++++++++
				if (endsamp > bufEndSamp && !panic) {
#ifdef DBUG
					cout << "re queueing inst " << (void *) Iptr << endl;
#endif
					rtQueue[busq].push(Iptr,rtQchunkStart+chunksamps);   // put back onto queue
				}
				else {
					pthread_mutex_lock(&bus_slot_lock);
					short t_count;
					short t_class = iBus->Class();
					short endbus = 999;  // Don't end yet
					switch (t_class) {
					case TO_AUX:
						t_count = iBus->auxout_count;
						endbus = iBus->auxout[t_count-1];
						break;
					case AUX_TO_AUX:
						t_count = iBus->auxout_count;
						endbus = iBus->auxout[t_count-1];
						break;
					case TO_AUX_AND_OUT:
						if (qStatus == TO_OUT) {
							t_count = iBus->out_count;
							endbus = iBus->out[t_count-1];			
						}
						else
							endbus = 1000;  /* can never equal this */
						break;
					case TO_OUT:
						t_count = iBus->out_count;
						endbus = iBus->out[t_count-1];
						break;
					default:
						cout << "ERROR (intraverse): unknown bus_class\n";
						break;
					}
					pthread_mutex_unlock(&bus_slot_lock);
					if (qStatus == t_class && bus == endbus) {
#ifdef DBUG
						cout << "unref'ing inst " << (void *) Iptr << endl;
#endif
						Iptr->unref();
					}
				}  // end rtQueue or unref ----------------------------------------

				// DJT:  not sure this check before new rtQchunkStart is necessary
				rtQSize = rtQueue[busq].getSize();
				if (rtQSize) {
					rtQchunkStart = rtQueue[busq].nextChunk(); /* FIXME:  crapping out */
					allQSize += rtQSize;                /* in RT situation sometimes */
				}
#ifdef DBUG
				cout << "rtQSize: " << rtQSize << endl;
				cout << "rtQchunkStart:  " << rtQchunkStart << endl;
				cout << "chunksamps:  " << chunksamps << endl;
				cout << "Iteration done==========\n";
#endif
			} // end Play elements on queue (insert back in if needed) -----------
		}  // end aux_pb_done --------------------------------------------------

		// Write buf to audio device - - - - - - - - - - - - - - - - - - - - -
#ifdef DBUG
		cout << "Writing samples----------\n";
		cout << "Q-rtQchunkStart:  " << rtQchunkStart << endl;
		cout << "bufEndSamp:  " << bufEndSamp << endl;
#endif

		// DT_PANIC_MOD
		if (!panic)
			rtsendsamps();

		// 	gettimeofday(&tv, &tz);
		// 	sec = (double)tv.tv_sec;
		// 	usec = (double)tv.tv_usec;
		// 	pthread_mutex_lock(&schedtime_lock);
		// 	baseTime = (sec * 1e6) + usec;
		elapsed += RTBUFSAMPS;	
		// 	pthread_mutex_unlock(&schedtime_lock);
		bufStartSamp += RTBUFSAMPS;
		bufEndSamp += RTBUFSAMPS;

		// zero the buffers
		clear_aux_buffers();
		clear_output_buffers();

		// read in an input buffer (if audio input is active)
		if (audio_on) { 
			// cout << "Reading data from audio port\n";
			// DT_PANIC_MOD
			if (!panic)
				rtgetsamps();
		}

		if (!rtInteractive) {  // Ending condition
			if ((rtHeap.getSize() == 0) && (allQSize == 0)) {
#ifdef ALLBUG
				cout << "heapSize:  " << rtHeap.getSize() << endl;
				cout << "rtQSize:  " << rtQSize << endl;
				cout << "PLAYEM = 0\n";
				cout << "The end\n\n";
#endif
				playEm = 0;
			}
		}
		else {
			// Check status from other threads
			if (run_status == RT_SHUTDOWN) {
				cout << "inTraverse():  shutting down" << endl;
				playEm = 0;
			}
			else if (run_status == RT_ERROR) {
				cout << "inTraverse():  shutting down due to error" << endl;
				playEm = 0;
			}
			else if (panic && run_status == RT_GOOD) {
				cout << "inTraverse():  panic mode finished" << endl;
				panic = NO;
			}
			// DT_PANIC_MOD
			if (panic && (rtHeap.getSize() == 0) && (allQSize == 0))
				run_status = RT_GOOD;
 		 
		}
	} // end playEm ----------------------------------------------------------

	if (rtsetparams_called) {
		if (print_is_on)
			cout << "closing ...\n";
		for (i = 0; i < ZERO_FRAMES_AFTER / RTBUFSAMPS; i++)
			rtsendzeros(0);  // send a buffer of zeros to audio output device
		close_audio_ports();
		rtreportstats();
		rtcloseout();
	}

	cout << "\n";
#ifdef ALLBUG
	cout << "EXITING inTraverse() FUNCTION *****\n";
	exit(0);
#endif

	return NULL;
}

