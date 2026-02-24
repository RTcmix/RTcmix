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
#include "BusSlot.h"
#include <RTcmix.h>
#include <heap/heap.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <algorithm>

#include "dbug.h"

/* -------------------------------------------- InstrumentBus::InstrumentBus --- */

InstrumentBus::InstrumentBus(int busID, int bufsamps)
    : mBusID(busID),
      mBufsamps(bufsamps),
      mBufferSize(bufsamps * INSTBUS_BUFFER_MULTIPLIER),
      mFramesProduced(RTcmix::bufStartSamp),
      mLastAdvancedBufStart(-1)
{
#ifdef IBUG
    printf("InstBus %d: created, bufferSize=%d frames (multiplier=%d)\n",
           mBusID, mBufferSize, INSTBUS_BUFFER_MULTIPLIER);
#endif
}


/* ------------------------------------------- InstrumentBus::~InstrumentBus --- */

InstrumentBus::~InstrumentBus()
{
    /* aux_buffer is freed by RTcmix::free_buffers() - nothing to do here */
}


/* -------------------------------------------------- InstrumentBus::reset --- */

void InstrumentBus::reset()
{
    mFramesProduced = 0;

    /* Clear aux_buffer (our ring buffer) */
    BufPtr buf = RTcmix::aux_buffer[mBusID];
    memset(buf, 0, sizeof(BUFTYPE) * mBufferSize);

    /* Reset all consumer cursors */
    std::for_each(mConsumers.begin(), mConsumers.end(),
        [](std::pair<Instrument* const, ConsumerState> &p) { p.second.framesConsumed = 0; });

#ifdef IBUG
    printf("InstBus %d: reset complete, %d consumers\n",
           mBusID, (int)mConsumers.size());
#endif
}


/* ---------------------------------------- InstrumentBus::advanceProduction --- */

void InstrumentBus::advanceProduction(int frames, FRAMETYPE currentBufStart)
{
    /* Idempotent per cycle: a bus visited in both TO_AUX and AUX_TO_AUX
     * calls this from each phase; the second call is a no-op. */
    if (mLastAdvancedBufStart == currentBufStart)
        return;

    mFramesProduced += frames;
    mLastAdvancedBufStart = currentBufStart;

#ifdef IBUG
    printf("InstBus %d: advanceProduction(%d), mFramesProduced now %lld\n",
           mBusID, frames, (long long)mFramesProduced);
#endif
}


/* -------------------------------- InstrumentBus::prepareForIntraverseWrite --- */

int InstrumentBus::prepareForIntraverseWrite(FRAMETYPE currentBufStart)
{
    /* If already produced this cycle (TO_AUX phase completed), the
     * second phase (AUX_TO_AUX) accumulates into the same region. */
    if (mLastAdvancedBufStart == currentBufStart) {
        int prevWriteStart = (int)((mFramesProduced - mBufsamps) % mBufferSize);
#ifdef IBUG
        printf("InstBus %d: prepareForIntraverseWrite (2nd phase), "
               "returning prevWriteStart=%d\n", mBusID, prevWriteStart);
#endif
        return prevWriteStart;
    }

    int writeStart = getWriteRegionStart();

#ifdef IBUG
    printf("InstBus %d: prepareForIntraverseWrite, writeStart=%d "
           "(mFramesProduced=%lld)\n",
           mBusID, writeStart, (long long)mFramesProduced);
#endif

    clearRegion(writeStart, mBufsamps);
    return writeStart;
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


/* -------------------------------------- InstrumentBus::hasRoomForProduction --- */

bool InstrumentBus::hasRoomForProduction(FRAMETYPE currentBufStart) const
{
    /* Already produced this cycle (e.g., TO_AUX phase completed) —
     * the second phase (AUX_TO_AUX) accumulates into the same region. */
    if (mLastAdvancedBufStart == currentBufStart)
        return true;

    /* Check whether producing mBufsamps more frames would overwrite any
     * consumer's unconsumed data.  With MULTIPLIER=1 (bufferSize == bufsamps),
     * this means framesAvailable must be 0 for all consumers. */
    for (std::map<Instrument*, ConsumerState>::const_iterator it = mConsumers.begin();
         it != mConsumers.end(); ++it) {
        FRAMETYPE avail = mFramesProduced - it->second.framesConsumed;
        if (avail + mBufsamps > mBufferSize) {
#ifdef IBUG
            printf("InstBus %d: hasRoomForProduction(%lld) = false "
                   "(consumer %p avail=%lld, bufsamps=%d, bufSize=%d)\n",
                   mBusID, (long long)currentBufStart,
                   it->first, (long long)avail, mBufsamps, mBufferSize);
#endif
            return false;
        }
    }
#ifdef IBUG
    printf("InstBus %d: hasRoomForProduction(%lld) = true\n",
           mBusID, (long long)currentBufStart);
#endif
    return true;
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
#ifdef IBUG
    int cycleCount = 0;
#endif
    while (framesAvailable(consumer) < requestedFrames) {
#ifdef IBUG
        printf("InstBus %d: need more frames, running production cycle %d\n",
               mBusID, ++cycleCount);
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
    bool panic = (RTcmix::getRunStatus() == RT_PANIC);

#ifdef BBUG
    printf("InstBus %d: writeStart=%d, mFramesProduced=%lld, localBufEnd=%lld\n",
           mBusID, writeStart, (long long)mFramesProduced, (long long)localBufEnd);
#endif

    clearRegion(writeStart, mBufsamps);

    int busCount = RTcmix::busCount;
    int busqs[2] = { mBusID, mBusID + busCount };
    std::vector<Instrument*> perQueue[2];
    bool anyInsts = false;

    for (int q = 0; q < 2; q++) {
        perQueue[q] = popAndPrepareWriters(
            busqs[q], mFramesProduced, localBufEnd, writeStart, mBufsamps, panic);
        /* Execute sequentially (pull path — no TaskManager).
         * runWriterCycle is always called from a TaskThread
         * (via pullFrames -> rtgetin -> Instrument::run -> exec).
         * Using TaskManager here would cause nested startAndWait,
         * where participatory waiting steals unrelated tasks from
         * the global stack, causing unbounded recursion. */
        if (!panic) {
            std::for_each(perQueue[q].begin(), perQueue[q].end(),
                [this](Instrument *Iptr) { Iptr->exec(BUS_AUX_OUT, mBusID); });
        }
        if (!perQueue[q].empty()) anyInsts = true;
    }

#ifdef MULTI_THREAD
    /* In MULTI_THREAD mode, addToBus() queues mix operations per-thread.
     * Even though writers execute sequentially, mixToBus() is still
     * needed to apply the queued operations to aux_buffer. */
    if (anyInsts) {
        RTcmix::mixToBus();
    }
#endif

    for (int q = 0; q < 2; q++) {
        requeueOrUnref(perQueue[q], localBufEnd, busqs[q], UNKNOWN, panic);
        RTcmix::rtQueue[busqs[q]].sort();
    }

    mFramesProduced += mBufsamps;

#ifdef IBUG
    printf("InstBus %d: mFramesProduced now %lld\n",
           mBusID, (long long)mFramesProduced);
#endif

#ifdef WBUG
    printf("EXITING InstrumentBus::runWriterCycle() for bus %d\n", mBusID);
#endif
}


/* --------------------------------- InstrumentBus::popAndPrepareWriters --- */

std::vector<Instrument*> InstrumentBus::popAndPrepareWriters(int busq,
                                                              FRAMETYPE timelineOrigin,
                                                              FRAMETYPE bufEnd,
                                                              int writeStart,
                                                              int maxFrames,
                                                              bool panic)
{
    RTQueue &queue = RTcmix::rtQueue[busq];
    std::vector<Instrument*> instsToRun;

#ifdef BBUG
    printf("popAndPrepareWriters: queue[%d] size=%d\n", busq, queue.getSize());
#endif

    while (queue.getSize() > 0) {
        FRAMETYPE rtQchunkStart = queue.nextChunk();
        if (rtQchunkStart >= bufEnd) {
#ifdef BBUG
            printf("popAndPrepareWriters: nextChunk %lld >= bufEnd %lld, "
                   "done with queue[%d]\n",
                   (long long)rtQchunkStart, (long long)bufEnd, busq);
#endif
            break;
        }

        Instrument *Iptr = queue.pop(&rtQchunkStart);
        Iptr->set_ichunkstart(rtQchunkStart);

        FRAMETYPE endsamp = Iptr->getendsamp();
        int offset = (int)(rtQchunkStart - timelineOrigin);

#ifdef IBUG
        printf("popAndPrepareWriters: popped inst %p [%s] from queue[%d], "
               "rtQchunkStart=%lld, endsamp=%lld, offset=%d\n",
               Iptr, Iptr->name(), busq,
               (long long)rtQchunkStart, (long long)endsamp, offset);
#endif

        if (offset < 0) {
#ifdef DBUG
            printf("popAndPrepareWriters: WARNING - scheduler behind queue, "
                   "offset=%d, adjusting\n", offset);
#endif
            endsamp += offset;
            offset = 0;
        }
        Iptr->set_output_offset(writeStart + offset);

        int chunksamps;
        if (endsamp < bufEnd)
            chunksamps = (int)(endsamp - rtQchunkStart);
        else
            chunksamps = (int)(bufEnd - rtQchunkStart);
        if (chunksamps > maxFrames) {
#ifdef DBUG
            printf("popAndPrepareWriters: ERROR - chunksamps %d > maxFrames %d, "
                   "limiting\n", chunksamps, maxFrames);
#endif
            chunksamps = maxFrames;
        }
        else if (chunksamps + offset > maxFrames) {
#ifdef DBUG
            printf("popAndPrepareWriters: ERROR - chunksamps+offset %d > maxFrames %d, "
                   "limiting\n", chunksamps + offset, maxFrames);
#endif
            chunksamps = maxFrames - offset;
        }

        Iptr->setchunk(chunksamps);

#ifdef IBUG
        printf("popAndPrepareWriters: inst %p [%s] output_offset=%d, chunksamps=%d\n",
               Iptr, Iptr->name(), writeStart + offset, chunksamps);
#endif

        if (!panic) {
            instsToRun.push_back(Iptr);
        }
#ifdef DBUG
        else {
            printf("popAndPrepareWriters: panic mode, skipping inst %p\n", Iptr);
        }
#endif
    }

    return instsToRun;
}


/* ---------------------------------------- InstrumentBus::requeueOrUnref --- */

void InstrumentBus::requeueOrUnref(std::vector<Instrument*> &instsToRun,
                                    FRAMETYPE bufEnd,
                                    int busq,
                                    IBusClass qStatus,
                                    bool panic)
{
    for (size_t i = 0; i < instsToRun.size(); i++) {
        Instrument *Iptr = instsToRun[i];
        FRAMETYPE endsamp = Iptr->getendsamp();
        FRAMETYPE rtQchunkStart = Iptr->get_ichunkstart();
        int chunksamps = Iptr->framesToRun();

        assert(busq >= 0);
        if (endsamp > bufEnd && !panic) {
#ifdef IBUG
            printf("requeueOrUnref: re-queuing inst %p [%s] on queue[%d] at %lld\n",
                   Iptr, Iptr->name(), busq,
                   (long long)(rtQchunkStart + chunksamps));
#endif
            RTcmix::rtQueue[busq].pushUnsorted(Iptr, rtQchunkStart + chunksamps);
        } else {
            const BusSlot *iBus = Iptr->getBusSlot();
            int inst_chunk_finished = Iptr->needsToRun();
            /* Unref when finished and either pull path (UNKNOWN) or
             * all buses for this instrument have played. */
            bool shouldUnref = inst_chunk_finished
                && (qStatus == UNKNOWN || qStatus == iBus->Class());
            if (shouldUnref) {
#ifdef IBUG
                printf("requeueOrUnref: unref'ing inst %p [%s]\n",
                       Iptr, Iptr->name());
#endif
                Iptr->unref();
            }
#ifdef IBUG
            else {
                printf("requeueOrUnref: inst %p [%s] not yet finished or "
                       "qStatus mismatch, keeping ref\n", Iptr, Iptr->name());
            }
#endif
        }
    }
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


/* ----------------------------------------- InstrumentBus::removeConsumer --- */

void InstrumentBus::removeConsumer(Instrument* inst)
{
    AutoLock al(mPullLock);
    std::map<Instrument*, ConsumerState>::iterator it = mConsumers.find(inst);
    if (it != mConsumers.end()) {
#ifdef IBUG
        printf("InstBus %d: removeConsumer(%p), framesConsumed=%lld, "
               "%d consumers remaining\n",
               mBusID, inst, (long long)it->second.framesConsumed,
               (int)mConsumers.size() - 1);
#endif
        mConsumers.erase(it);
    }
}
