/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/*
 * InstrumentBus.cpp - Pull model implementation for elastic audio bus buffering
 *
 * See InstrumentBus.h for design overview.
 */

#include "InstrumentBus.h"
#include "Instrument.h"
#include <RTcmix.h>
#include <heap/heap.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef MULTI_THREAD
#include "TaskManager.h"
#endif

/* Debug macros - match pattern from intraverse.cpp */
#undef BBUG      /* Verbose bus debugging */
#undef WBUG      /* "Where we are" prints */
#undef IBUG      /* Instrument and InstrumentBus debugging */
#undef DBUG      /* General debug */
#undef ALLBUG    /* All debug output */

#ifdef ALLBUG
#define BBUG
#define WBUG
#define IBUG
#define DBUG
#endif

/* -------------------------------------------- InstrumentBus::InstrumentBus --- */

InstrumentBus::InstrumentBus(int busID, int bufsamps)
    : mBusID(busID),
      mBufsamps(bufsamps),
      mBufferSize(bufsamps * INSTBUS_BUFFER_MULTIPLIER),
      mFramesProduced(RTcmix::bufStartSamp)
#ifdef MULTI_THREAD
      , mTaskManager(NULL)
#endif
{
#ifdef WBUG
    printf("ENTERING InstrumentBus::InstrumentBus(busID=%d, bufsamps=%d)\n",
           busID, bufsamps);
#endif

#ifdef IBUG
    printf("InstBus %d: created, bufferSize=%d frames (multiplier=%d)\n",
           mBusID, mBufferSize, INSTBUS_BUFFER_MULTIPLIER);
#endif

#ifdef WBUG
    printf("EXITING InstrumentBus::InstrumentBus()\n");
#endif
}


/* ------------------------------------------- InstrumentBus::~InstrumentBus --- */

InstrumentBus::~InstrumentBus()
{
#ifdef WBUG
    printf("ENTERING InstrumentBus::~InstrumentBus(busID=%d)\n", mBusID);
#endif

    /* aux_buffer is freed by RTcmix::free_buffers() - nothing to do here */

#ifdef WBUG
    printf("EXITING InstrumentBus::~InstrumentBus()\n");
#endif
}


/* -------------------------------------------------- InstrumentBus::reset --- */

void InstrumentBus::reset()
{
#ifdef WBUG
    printf("ENTERING InstrumentBus::reset(busID=%d)\n", mBusID);
#endif

    mFramesProduced = 0;

    /* Clear aux_buffer (our ring buffer) */
    BufPtr buf = RTcmix::aux_buffer[mBusID];
    memset(buf, 0, sizeof(BUFTYPE) * mBufferSize);

    /* Reset all consumer cursors */
    for (std::map<Instrument*, ConsumerState>::iterator it = mConsumers.begin();
         it != mConsumers.end(); ++it) {
        it->second.framesConsumed = 0;
    }

#ifdef IBUG
    printf("InstBus %d: reset complete, %d consumers\n",
           mBusID, (int)mConsumers.size());
#endif

#ifdef WBUG
    printf("EXITING InstrumentBus::reset()\n");
#endif
}


/* ---------------------------------------------- InstrumentBus::addConsumer --- */

void InstrumentBus::addConsumer(Instrument* inst)
{
    /* Lock required: addConsumer is called from the parser thread
     * (via set_bus_config) while pullFrames/framesAvailable may be
     * accessing mConsumers from TaskThreads. */
    AutoLock al(mPullLock);

    /* Initialize framesConsumed to the later of mFramesProduced and
     * the global clock.  On an idle bus mFramesProduced is stale,
     * so the global clock gives the correct starting point.  On an
     * active bus mFramesProduced is at or ahead of the global clock
     * and is the right value. */
    FRAMETYPE startFrame = mFramesProduced;
    if (RTcmix::bufStartSamp > startFrame)
        startFrame = RTcmix::bufStartSamp;

    ConsumerState state;
    state.framesConsumed = startFrame;
    mConsumers[inst] = state;

#ifdef IBUG
    printf("InstBus %d: addConsumer(%p [%s]), framesConsumed=%lld "
           "(mFramesProduced=%lld, bufStartSamp=%lld), now %d consumers\n",
           mBusID, inst, inst->name(), (long long)startFrame,
           (long long)mFramesProduced, (long long)RTcmix::bufStartSamp,
           (int)mConsumers.size());
#endif
}


/* ------------------------------------------ InstrumentBus::framesAvailable --- */

int InstrumentBus::framesAvailable(Instrument* consumer) const
{
    std::map<Instrument*, ConsumerState>::const_iterator it = mConsumers.find(consumer);
    assert(it != mConsumers.end());

    /* Available = produced - consumed */
    FRAMETYPE available = mFramesProduced - it->second.framesConsumed;

#ifdef BBUG
    printf("InstBus %d: framesAvailable(consumer=%p) = %lld "
           "(produced=%lld, consumed=%lld)\n",
           mBusID, consumer, (long long)available,
           (long long)mFramesProduced, (long long)it->second.framesConsumed);
#endif

    /* Clamp to buffer size (ring buffer constraint) */
    if (available > mBufferSize) {
        available = mBufferSize;
    }

    return (int)available;
}


/* ------------------------------------------ InstrumentBus::getReadPosition --- */

int InstrumentBus::getReadPosition(Instrument* consumer, int frames)
{
    std::map<Instrument*, ConsumerState>::iterator it = mConsumers.find(consumer);
    assert(it != mConsumers.end());

    ConsumerState& state = it->second;
    int readPos = (int)(state.framesConsumed % mBufferSize);

#ifdef IBUG
    printf("InstBus %d: getReadPosition(consumer=%p, frames=%d) "
           "readPos=%d, framesConsumed=%lld -> %lld\n",
           mBusID, consumer, frames, readPos,
           (long long)state.framesConsumed,
           (long long)(state.framesConsumed + frames));
#endif

    state.framesConsumed += frames;

    return readPos;
}


/* --------------------------------------------- InstrumentBus::pullFrames --- */

int InstrumentBus::pullFrames(Instrument* consumer, int requestedFrames)
{
#ifdef WBUG
    printf("ENTERING InstrumentBus::pullFrames(bus=%d, consumer=%p, frames=%d)\n",
           mBusID, consumer, requestedFrames);
#endif

    /* Serialize production per bus. In the push model, bus production was
     * inherently serial (phase ordering). Multiple consumers may call
     * pullFrames concurrently from different TaskThreads; without this lock,
     * concurrent runWriterCycle calls corrupt the rtQueue (std::vector). */
    AutoLock al(mPullLock);  // Required: serializes runWriterCycle per bus

    /* If the bus has been idle, mFramesProduced may be behind the
     * consumer's framesConsumed (which was set to bufStartSamp in
     * addConsumer).  Fast-forward the bus so runWriterCycle pops
     * instruments at the correct time.  Both counters stay in sync
     * so no phantom frames are created. */
    std::map<Instrument*, ConsumerState>::iterator cit = mConsumers.find(consumer);
    assert(cit != mConsumers.end());
    if (mFramesProduced < cit->second.framesConsumed) {
#ifdef IBUG
        printf("InstBus %d: fast-forward mFramesProduced %lld -> %lld "
               "(consumer %p framesConsumed)\n",
               mBusID, (long long)mFramesProduced,
               (long long)cit->second.framesConsumed, consumer);
#endif
        mFramesProduced = cit->second.framesConsumed;
    }

#ifdef IBUG
    printf("InstBus %d: consumer %p requesting %d frames, available=%d\n",
           mBusID, consumer, requestedFrames, framesAvailable(consumer));
#endif

    /* Run production cycles until we have enough frames */
    int cycleCount = 0;
    while (framesAvailable(consumer) < requestedFrames) {
#ifdef IBUG
        printf("InstBus %d: need more frames, running production cycle %d\n",
               mBusID, ++cycleCount);
#else
        (void)cycleCount;
#endif
        runWriterCycle();
    }

#ifdef IBUG
    if (cycleCount > 0) {
        printf("InstBus %d: production complete after %d cycles, "
               "available=%d\n",
               mBusID, cycleCount, framesAvailable(consumer));
    }
#endif

    int readPos = getReadPosition(consumer, requestedFrames);

#ifdef WBUG
    printf("EXITING InstrumentBus::pullFrames(bus=%d) returning readPos=%d\n",
           mBusID, readPos);
#endif

    return readPos;
}


/* ----------------------------------------- InstrumentBus::runWriterCycle --- */

void InstrumentBus::runWriterCycle()
{
#ifdef WBUG
    printf("ENTERING InstrumentBus::runWriterCycle() for bus %d\n", mBusID);
#endif

    int writeStart = getWriteRegionStart();
    FRAMETYPE localBufEnd = mFramesProduced + mBufsamps;

#ifdef BBUG
    printf("InstBus %d: writeStart=%d, mFramesProduced=%lld, localBufEnd=%lld\n",
           mBusID, writeStart, (long long)mFramesProduced, (long long)localBufEnd);
#endif

    /* 1. Clear the write region */
    clearRegion(writeStart, mBufsamps);

#ifdef BBUG
    printf("InstBus %d: cleared region [%d, %d)\n",
           mBusID, writeStart, writeStart + mBufsamps);
#endif

    int busCount = RTcmix::busCount;
    int busqs[2] = { mBusID, mBusID + busCount };  /* TO_AUX, AUX_TO_AUX */

    struct QueuedInst {
        Instrument* inst;
        int busq;
    };
    std::vector<QueuedInst> queuedInsts;

    bool panic = (RTcmix::getRunStatus() == RT_PANIC);

    /* 2. Pop and exec from both TO_AUX and AUX_TO_AUX queues */
    for (int q = 0; q < 2; q++) {
        int busq = busqs[q];
        RTQueue &queue = RTcmix::rtQueue[busq];

#ifdef BBUG
        printf("InstBus %d: checking rtQueue[%d] (size=%d, %s)\n",
               mBusID, busq, queue.getSize(),
               q == 0 ? "TO_AUX" : "AUX_TO_AUX");
#endif

        while (queue.getSize() > 0) {
            FRAMETYPE rtQchunkStart = queue.nextChunk();
            if (rtQchunkStart >= localBufEnd) {
#ifdef BBUG
                printf("InstBus %d: nextChunk %lld >= localBufEnd %lld, "
                       "done with queue[%d]\n",
                       mBusID, (long long)rtQchunkStart,
                       (long long)localBufEnd, busq);
#endif
                break;
            }

            Instrument *Iptr = queue.pop(&rtQchunkStart);
            Iptr->set_ichunkstart(rtQchunkStart);

            FRAMETYPE endsamp = Iptr->getendsamp();
            int offset = (int)(rtQchunkStart - mFramesProduced);

#ifdef IBUG
            printf("InstBus %d: popped inst %p [%s] from queue[%d], "
                   "rtQchunkStart=%lld, endsamp=%lld, offset=%d\n",
                   mBusID, Iptr, Iptr->name(), busq,
                   (long long)rtQchunkStart, (long long)endsamp, offset);
#endif

            if (offset < 0) {
#ifdef DBUG
                printf("InstBus %d: WARNING - scheduler behind queue, "
                       "offset=%d, adjusting\n", mBusID, offset);
#endif
                endsamp += offset;
                offset = 0;
            }
            Iptr->set_output_offset(writeStart + offset);

            int chunksamps;
            if (endsamp < localBufEnd)
                chunksamps = (int)(endsamp - rtQchunkStart);
            else
                chunksamps = (int)(localBufEnd - rtQchunkStart);
            if (chunksamps > mBufsamps) {
#ifdef DBUG
                printf("InstBus %d: ERROR - chunksamps %d > bufsamps %d, "
                       "limiting\n", mBusID, chunksamps, mBufsamps);
#endif
                chunksamps = mBufsamps;
            }
            else if (chunksamps + offset > mBufsamps) {
#ifdef DBUG
                printf("InstBus %d: ERROR - chunksamps+offset %d > bufsamps %d, "
                       "limiting\n", mBusID, chunksamps + offset, mBufsamps);
#endif
                chunksamps = mBufsamps - offset;
            }

            Iptr->setchunk(chunksamps);

#ifdef IBUG
            printf("InstBus %d: inst %p [%s] output_offset=%d, chunksamps=%d\n",
                   mBusID, Iptr, Iptr->name(), writeStart + offset, chunksamps);
#endif

            QueuedInst qi = { Iptr, busq };
            queuedInsts.push_back(qi);

            if (!panic) {
                /* Execute writers sequentially on the current thread.
                 * runWriterCycle is always called from a TaskThread
                 * (via pullFrames -> rtgetin -> Instrument::run -> exec).
                 * Using TaskManager here would cause nested startAndWait,
                 * where participatory waiting steals unrelated tasks from
                 * the global stack, causing unbounded recursion. */
                Iptr->exec(BUS_AUX_OUT, mBusID);
#ifdef IBUG
                printf("InstBus %d: executed inst %p [%s] directly\n",
                       mBusID, Iptr, Iptr->name());
#endif
            }
#ifdef DBUG
            else {
                printf("InstBus %d: panic mode, skipping exec for inst %p\n",
                       mBusID, Iptr);
            }
#endif
        }
    }

#ifdef MULTI_THREAD
    /* In MULTI_THREAD mode, addToBus() queues mix operations per-thread.
     * Even though writers execute sequentially, mixToBus() is still
     * needed to apply the queued operations to aux_buffer. */
    if (!queuedInsts.empty()) {
        RTcmix::mixToBus();
    }
#endif

    /* 3. Re-queue or unref after all writers have completed */
    for (size_t i = 0; i < queuedInsts.size(); i++) {
        Instrument *Iptr = queuedInsts[i].inst;
        int busq = queuedInsts[i].busq;
        FRAMETYPE endsamp = Iptr->getendsamp();
        FRAMETYPE rtQchunkStart = Iptr->get_ichunkstart();
        int chunksamps = Iptr->framesToRun();

        if (endsamp > localBufEnd && !panic) {
#ifdef IBUG
            printf("InstBus %d: re-queuing inst %p [%s] on queue[%d] at %lld\n",
                   mBusID, Iptr, Iptr->name(), busq,
                   (long long)(rtQchunkStart + chunksamps));
#endif
            RTcmix::rtQueue[busq].pushUnsorted(Iptr, rtQchunkStart + chunksamps);
        } else {
            int inst_chunk_finished = Iptr->needsToRun();
            if (inst_chunk_finished) {
#ifdef IBUG
                printf("InstBus %d: unref'ing inst %p [%s]\n",
                       mBusID, Iptr, Iptr->name());
#endif
                Iptr->unref();
            }
#ifdef IBUG
            else {
                printf("InstBus %d: inst %p [%s] not yet finished, "
                       "keeping ref\n", mBusID, Iptr, Iptr->name());
            }
#endif
        }
    }

    /* Sort queues that received re-queued instruments */
    for (int q = 0; q < 2; q++) {
        RTcmix::rtQueue[busqs[q]].sort();
    }

    /* 4. Advance production counter */
    mFramesProduced += mBufsamps;

#ifdef IBUG
    printf("InstBus %d: mFramesProduced now %lld\n",
           mBusID, (long long)mFramesProduced);
#endif

#ifdef WBUG
    printf("EXITING InstrumentBus::runWriterCycle() for bus %d\n", mBusID);
#endif
}


/* -------------------------------------------- InstrumentBus::clearRegion --- */

void InstrumentBus::clearRegion(int startFrame, int numFrames)
{
    BufPtr buf = RTcmix::aux_buffer[mBusID];

    /* Handle wrap-around (aux buses are mono) */
    int endFrame = startFrame + numFrames;

    if (endFrame <= mBufferSize) {
        /* No wrap - single clear */
        memset(&buf[startFrame], 0, sizeof(BUFTYPE) * numFrames);
    } else {
        /* Wrap around - two clears */
        int firstPart = mBufferSize - startFrame;
        int secondPart = numFrames - firstPart;

        memset(&buf[startFrame], 0, sizeof(BUFTYPE) * firstPart);
        memset(&buf[0], 0, sizeof(BUFTYPE) * secondPart);

#ifdef BBUG
        printf("InstBus %d: clearRegion wrapped: [%d, %d) + [0, %d)\n",
               mBusID, startFrame, mBufferSize, secondPart);
#endif
    }
}


/* ----------------------------------------- InstrumentBus::copyToConsumer --- */

void InstrumentBus::copyToConsumer(BufPtr dest, int readPos, int numFrames)
{
    BufPtr buf = RTcmix::aux_buffer[mBusID];

    /* Handle wrap-around (aux buses are mono) */
    int endPos = readPos + numFrames;

    if (endPos <= mBufferSize) {
        /* No wrap - single copy */
        memcpy(dest, &buf[readPos], sizeof(BUFTYPE) * numFrames);
    } else {
        /* Wrap around - two copies */
        int firstPart = mBufferSize - readPos;
        int secondPart = numFrames - firstPart;

        memcpy(dest, &buf[readPos], sizeof(BUFTYPE) * firstPart);
        memcpy(&dest[firstPart], &buf[0], sizeof(BUFTYPE) * secondPart);

#ifdef BBUG
        printf("InstBus %d: copyToConsumer wrapped: [%d, %d) + [0, %d)\n",
               mBusID, readPos, mBufferSize, secondPart);
#endif
    }
}


/* -------------------------------------- InstrumentBus::getConsumerCount --- */

int InstrumentBus::getConsumerCount() const
{
    return (int)mConsumers.size();
}
