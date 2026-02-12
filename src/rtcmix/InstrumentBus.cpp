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
#define IBUG      /* Instrument and InstrumentBus debugging */
#define DBUG      /* General debug */
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
      mWritePosition(0),
      mFramesProduced(0)
#ifdef MULTI_THREAD
      , mTaskManager(NULL)
#endif
{
#ifdef WBUG
    printf("ENTERING InstrumentBus::InstrumentBus(busID=%d, bufsamps=%d)\n",
           busID, bufsamps);
#endif

    /* Ring buffer uses aux_buffer directly - no allocation here.
     * aux_buffer is allocated larger (mBufferSize) by InstrumentBusManager.
     */

#ifdef IBUG
    printf("InstBus %d: using aux_buffer as ring buffer, %d frames\n",
           mBusID, mBufferSize);
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

    mWritePosition = 0;
    mFramesProduced = 0;

    /* Clear aux_buffer (our ring buffer) */
    BufPtr buf = RTcmix::aux_buffer[mBusID];
    assert(buf != NULL);
    memset(buf, 0, sizeof(BUFTYPE) * mBufferSize);

    /* Reset all consumer cursors */
    for (std::map<Instrument*, ConsumerState>::iterator it = mConsumers.begin();
         it != mConsumers.end(); ++it) {
        it->second.readCursor = 0;
        it->second.framesConsumed = 0;
    }

    /* Note: We don't clear the writers array - instruments are still registered */

#ifdef IBUG
    printf("InstBus %d: reset complete, %d writers, %d consumers\n",
           mBusID, (int)mWriters.size(), (int)mConsumers.size());
#endif

#ifdef WBUG
    printf("EXITING InstrumentBus::reset()\n");
#endif
}


/* ----------------------------------------------- InstrumentBus::addWriter --- */

void InstrumentBus::addWriter(Instrument* inst)
{
#ifdef IBUG
    printf("InstBus %d: addWriter(%p [%s])\n", mBusID, inst, inst->name());
#endif

    mWriters.push_back(inst);

#ifdef IBUG
    printf("InstBus %d: now has %d writers\n", mBusID, (int)mWriters.size());
#endif
}


/* -------------------------------------------- InstrumentBus::removeWriter --- */

void InstrumentBus::removeWriter(Instrument* inst)
{
#ifdef IBUG
    printf("InstBus %d: removeWriter(%p [%s])\n", mBusID, inst, inst->name());
#endif

    for (std::vector<Instrument*>::iterator it = mWriters.begin();
         it != mWriters.end(); ++it) {
        if (*it == inst) {
            mWriters.erase(it);

#ifdef IBUG
            printf("InstBus %d: removed writer, now has %d writers\n",
                   mBusID, (int)mWriters.size());
#endif
            return;
        }
    }

#ifdef DBUG
    printf("InstBus %d: WARNING - removeWriter called for unknown inst %p\n",
           mBusID, inst);
#endif
}


/* ---------------------------------------------- InstrumentBus::addConsumer --- */

void InstrumentBus::addConsumer(Instrument* inst)
{
    ConsumerState state;
    state.readCursor = mWritePosition;
    state.framesConsumed = 0;
    mConsumers[inst] = state;

#ifdef IBUG
    printf("InstBus %d: addConsumer(%p [%s]), now %d consumers\n",
           mBusID, inst, inst->name(), (int)mConsumers.size());
#endif
}


/* ------------------------------------------ InstrumentBus::framesAvailable --- */

int InstrumentBus::framesAvailable(Instrument* consumer) const
{
    std::map<Instrument*, ConsumerState>::const_iterator it = mConsumers.find(consumer);
    assert(it != mConsumers.end());

    /* Available = produced - consumed */
    FRAMETYPE available = mFramesProduced - it->second.framesConsumed;

#ifdef IBUG
    printf("InstBus %d: framesAvailable(consumer=%p) = %lld (produced=%lld, consumed=%lld)\n",
           mBusID, consumer, available, mFramesProduced, it->second.framesConsumed);
#endif

    /* Clamp to buffer size (ring buffer constraint) */
    if (available > mBufferSize) {
        available = mBufferSize;
    }

    return (int)available;
}


/* --------------------------------------------- InstrumentBus::pullFrames --- */

int InstrumentBus::pullFrames(Instrument* consumer, int requestedFrames)
{
#ifdef WBUG
    printf("ENTERING InstrumentBus::pullFrames(bus=%d, consumer=%p, frames=%d)\n",
           mBusID, consumer, requestedFrames);
#endif

    std::map<Instrument*, ConsumerState>::iterator it = mConsumers.find(consumer);
    assert(it != mConsumers.end());

    ConsumerState& state = it->second;
    int readPos = state.readCursor;

#ifdef IBUG
    printf("InstBus %d: consumer %p requesting %d frames, available=%d\n",
           mBusID, consumer, requestedFrames, framesAvailable(consumer));
#endif

    /* Run production cycles until we have enough frames */
    int cycleCount = 0;
    while (framesAvailable(consumer) < requestedFrames) {
        /* Check for no writers - would loop forever */
        if (mWriters.empty()) {
#ifdef DBUG
            printf("InstBus %d: pullFrames() no writers, aux_buffer is zeros\n",
                   mBusID);
#endif
            /* aux_buffer should already be zeroed, just return the position */
            break;
        }

#ifdef IBUG
        printf("InstBus %d: need more frames, running cycle %d\n",
               mBusID, ++cycleCount);
#else
        (void)cycleCount;  /* suppress unused variable warning */
#endif
        runWriterCycle();
    }

    /* Advance this consumer's read cursor */
#ifdef IBUG
    int oldPos = state.readCursor;
#endif
    state.readCursor = (state.readCursor + requestedFrames) % mBufferSize;
    state.framesConsumed += requestedFrames;

#ifdef IBUG
    printf("InstBus %d: consumer %p readCursor %d -> %d, consumed=%lld\n",
           mBusID, consumer, oldPos, state.readCursor, state.framesConsumed);
#endif

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

    /* 1. Clear the ring buffer region we're about to write */
    clearRegion(mWritePosition, mBufsamps);

#ifdef IBUG
    printf("InstBus %d: cleared region [%d, %d)\n",
           mBusID, mWritePosition, mWritePosition + mBufsamps);
#endif

#ifdef MULTI_THREAD
    mActiveWriters.clear();
#endif

    /* 2. Execute all writers */
    for (size_t i = 0; i < mWriters.size(); ++i) {
        Instrument* inst = mWriters[i];

        /* Set up instrument for this chunk.
         * output_offset is set to mWritePosition so addToBus writes
         * at the correct ring buffer position.
         */
        inst->setchunk(mBufsamps);
        inst->set_output_offset(mWritePosition);

#ifdef IBUG
        printf("InstBus %d: processing inst %p [%s]\n",
               mBusID, inst, inst->name());
#endif

#ifdef MULTI_THREAD
        if (mTaskManager != NULL) {
            mActiveWriters.push_back(inst);
            mTaskManager->addTask<Instrument, int, BusType, int, &Instrument::exec>
                (inst, BUS_AUX_OUT, mBusID);
        } else {
            /* Fallback to sequential if no TaskManager */
            inst->exec(BUS_AUX_OUT, mBusID);
        }
#else
        /* Single-threaded: execute directly */
        inst->exec(BUS_AUX_OUT, mBusID);
#endif
    }

#ifdef MULTI_THREAD
    /* 3. Wait for all writers to complete */
    if (mTaskManager != NULL && !mActiveWriters.empty()) {
#ifdef DBUG
        printf("InstBus %d: waiting for %d instrument tasks\n",
               mBusID, (int)mActiveWriters.size());
#endif
        mTaskManager->waitForTasks(mActiveWriters);

        /* 4. Merge per-thread accumulations into ring buffer */
        RTcmix::mixToBus();
    }
#endif

    /* 5. Advance write position */
    mWritePosition = (mWritePosition + mBufsamps) % mBufferSize;
    mFramesProduced += mBufsamps;

#ifdef IBUG
    printf("InstBus %d: writePosition now %d, framesProduced=%lld\n",
           mBusID, mWritePosition, mFramesProduced);
#endif

#ifdef WBUG
    printf("EXITING InstrumentBus::runWriterCycle() for bus %d\n", mBusID);
#endif
}


/* -------------------------------------------- InstrumentBus::clearRegion --- */

void InstrumentBus::clearRegion(int startFrame, int numFrames)
{
    BufPtr buf = RTcmix::aux_buffer[mBusID];
    assert(buf != NULL);

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
    }
}


/* ----------------------------------------- InstrumentBus::copyToConsumer --- */

void InstrumentBus::copyToConsumer(BufPtr dest, int readPos, int numFrames)
{
    BufPtr buf = RTcmix::aux_buffer[mBusID];
    assert(buf != NULL);

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
    }
}


/* ---------------------------------------- InstrumentBus::getWriterCount --- */

int InstrumentBus::getWriterCount() const
{
    return (int)mWriters.size();
}


/* -------------------------------------- InstrumentBus::getConsumerCount --- */

int InstrumentBus::getConsumerCount() const
{
    return (int)mConsumers.size();
}