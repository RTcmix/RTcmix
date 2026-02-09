/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/*
 * Tier.cpp - Tier-based pull model implementation
 *
 * See Tier.h for design overview.
 */

#include "Tier.h"
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
#undef TBUG      /* Tier-specific debugging */
#undef WBUG      /* "Where we are" prints */
#undef IBUG      /* Instrument debugging */
#undef DBUG      /* General debug */
#undef ALLBUG    /* All debug output */

#ifdef ALLBUG
#define TBUG
#define WBUG
#define IBUG
#define DBUG
#endif

/* Initial capacity for dynamic arrays */
#define INITIAL_WRITER_CAPACITY 8


/* ------------------------------------------------------------ Tier::Tier --- */

Tier::Tier(int busID, int numChannels, int bufferSize, int bufsamps)
    : mBusID(busID),
      mNumChannels(numChannels),
      mBufsamps(bufsamps),
      mRingBuffer(NULL),
      mBufferSize(0),
      mWritePosition(0),
      mFramesProduced(0),
      mWriters(NULL),
      mWriterCount(0),
      mWriterCapacity(0)
#ifdef MULTI_THREAD
      , mTaskManager(NULL)
#endif
{
#ifdef WBUG
    printf("ENTERING Tier::Tier(busID=%d, chans=%d, bufSize=%d, bufsamps=%d)\n",
           busID, numChannels, bufferSize, bufsamps);
#endif

    /* Calculate buffer size if not specified */
    if (bufferSize <= 0) {
        mBufferSize = bufsamps * TIER_BUFFER_MULTIPLIER;
    } else {
        mBufferSize = bufferSize;
    }

    /* Allocate ring buffer (interleaved) */
    mRingBuffer = new BUFTYPE[mBufferSize * mNumChannels];
    memset(mRingBuffer, 0, sizeof(BUFTYPE) * mBufferSize * mNumChannels);

    /* Allocate writer array */
    mWriterCapacity = INITIAL_WRITER_CAPACITY;
    mWriters = new Instrument*[mWriterCapacity];

#ifdef TBUG
    printf("Tier %d: allocated ring buffer %d frames x %d chans = %d samples\n",
           mBusID, mBufferSize, mNumChannels, mBufferSize * mNumChannels);
#endif

#ifdef WBUG
    printf("EXITING Tier::Tier()\n");
#endif
}


/* ----------------------------------------------------------- Tier::~Tier --- */

Tier::~Tier()
{
#ifdef WBUG
    printf("ENTERING Tier::~Tier(busID=%d)\n", mBusID);
#endif

    delete[] mRingBuffer;
    delete[] mWriters;

#ifdef WBUG
    printf("EXITING Tier::~Tier()\n");
#endif
}


/* ---------------------------------------------------------- Tier::reset --- */

void Tier::reset()
{
#ifdef WBUG
    printf("ENTERING Tier::reset(busID=%d)\n", mBusID);
#endif

    mWritePosition = 0;
    mFramesProduced = 0;

    /* Clear ring buffer */
    memset(mRingBuffer, 0, sizeof(BUFTYPE) * mBufferSize * mNumChannels);

    /* Reset all consumer cursors */
    for (std::map<Instrument*, ConsumerState>::iterator it = mConsumers.begin();
         it != mConsumers.end(); ++it) {
        it->second.readCursor = 0;
        it->second.framesConsumed = 0;
    }

    /* Note: We don't clear the writers array - instruments are still registered */

#ifdef TBUG
    printf("Tier %d: reset complete, %d writers, %d consumers\n",
           mBusID, mWriterCount, (int)mConsumers.size());
#endif

#ifdef WBUG
    printf("EXITING Tier::reset()\n");
#endif
}


/* ------------------------------------------------------- Tier::addWriter --- */

void Tier::addWriter(Instrument* inst)
{
#ifdef IBUG
    printf("Tier %d: addWriter(%p [%s])\n", mBusID, inst, inst->name());
#endif

    if (mWriterCount >= mWriterCapacity) {
        growWriters();
    }

    mWriters[mWriterCount++] = inst;

#ifdef TBUG
    printf("Tier %d: now has %d writers\n", mBusID, mWriterCount);
#endif
}


/* ---------------------------------------------------- Tier::removeWriter --- */

void Tier::removeWriter(Instrument* inst)
{
#ifdef IBUG
    printf("Tier %d: removeWriter(%p [%s])\n", mBusID, inst, inst->name());
#endif

    for (int i = 0; i < mWriterCount; ++i) {
        if (mWriters[i] == inst) {
            /* Shift remaining writers down */
            for (int j = i; j < mWriterCount - 1; ++j) {
                mWriters[j] = mWriters[j + 1];
            }
            --mWriterCount;

#ifdef TBUG
            printf("Tier %d: removed writer, now has %d writers\n",
                   mBusID, mWriterCount);
#endif
            return;
        }
    }

#ifdef DBUG
    printf("Tier %d: WARNING - removeWriter called for unknown inst %p\n",
           mBusID, inst);
#endif
}


/* ------------------------------------------------------ Tier::addConsumer --- */

void Tier::addConsumer(Instrument* inst)
{
    ConsumerState state;
    state.readCursor = mWritePosition;
    state.framesConsumed = 0;
    mConsumers[inst] = state;

#ifdef TBUG
    printf("Tier %d: addConsumer(%p [%s]), now %d consumers\n",
           mBusID, inst, inst->name(), (int)mConsumers.size());
#endif
}


/* -------------------------------------------------- Tier::framesAvailable --- */

int Tier::framesAvailable(Instrument* consumer) const
{
    std::map<Instrument*, ConsumerState>::const_iterator it = mConsumers.find(consumer);
    if (it == mConsumers.end()) {
#ifdef DBUG
        printf("Tier %d: framesAvailable() unknown consumer %p\n",
               mBusID, consumer);
#endif
        return 0;
    }

    /* Available = produced - consumed */
    FRAMETYPE available = mFramesProduced - it->second.framesConsumed;

#ifdef TBUG
    printf("Tier %d: framesAvailable(consumer=%p) = %lld (produced=%lld, consumed=%lld)\n",
           mBusID, consumer, available, mFramesProduced, it->second.framesConsumed);
#endif

    /* Clamp to buffer size (ring buffer constraint) */
    if (available > mBufferSize) {
        available = mBufferSize;
    }

    return (int)available;
}


/* ----------------------------------------------------- Tier::pullFrames --- */

int Tier::pullFrames(Instrument* consumer, int requestedFrames, BufPtr dest)
{
#ifdef WBUG
    printf("ENTERING Tier::pullFrames(bus=%d, consumer=%p, frames=%d)\n",
           mBusID, consumer, requestedFrames);
#endif

    std::map<Instrument*, ConsumerState>::iterator it = mConsumers.find(consumer);
    if (it == mConsumers.end()) {
#ifdef DBUG
        printf("Tier %d: pullFrames() unknown consumer %p\n",
               mBusID, consumer);
#endif
        return 0;
    }

    ConsumerState& state = it->second;

#ifdef TBUG
    printf("Tier %d: consumer %p requesting %d frames, available=%d\n",
           mBusID, consumer, requestedFrames, framesAvailable(consumer));
#endif

    /* Run production cycles until we have enough frames */
    int cycleCount = 0;
    while (framesAvailable(consumer) < requestedFrames) {
        /* Check for no writers - would loop forever */
        if (mWriterCount == 0) {
#ifdef DBUG
            printf("Tier %d: pullFrames() no writers, returning zeros\n",
                   mBusID);
#endif
            /* Fill with zeros and return */
            memset(dest, 0, sizeof(BUFTYPE) * requestedFrames * mNumChannels);
            return requestedFrames;
        }

#ifdef TBUG
        printf("Tier %d: need more frames, running cycle %d\n",
               mBusID, ++cycleCount);
#endif
        runWriterCycle();
    }

#ifdef TBUG
    printf("Tier %d: copying %d frames from readPos=%d to consumer %p\n",
           mBusID, requestedFrames, state.readCursor, consumer);
#endif

    /* Copy requested frames to consumer's buffer */
    copyToConsumer(dest, state.readCursor, requestedFrames);

    /* Advance this consumer's read cursor */
    int oldPos = state.readCursor;
    state.readCursor = (state.readCursor + requestedFrames) % mBufferSize;
    state.framesConsumed += requestedFrames;

#ifdef TBUG
    printf("Tier %d: consumer %p readCursor %d -> %d, consumed=%lld\n",
           mBusID, consumer, oldPos, state.readCursor, state.framesConsumed);
#endif

#ifdef WBUG
    printf("EXITING Tier::pullFrames(bus=%d) returning %d\n",
           mBusID, requestedFrames);
#endif

    return requestedFrames;
}


/* ------------------------------------------------- Tier::runWriterCycle --- */

void Tier::runWriterCycle()
{
#ifdef WBUG
    printf("ENTERING Tier::runWriterCycle() for bus %d\n", mBusID);
#endif

    /* 1. Clear the ring buffer region we're about to write */
    clearRegion(mWritePosition, mBufsamps);

#ifdef TBUG
    printf("Tier %d: cleared region [%d, %d)\n",
           mBusID, mWritePosition, mWritePosition + mBufsamps);
#endif

#ifdef MULTI_THREAD
    mActiveWriters.clear();
#endif

    /* 2. Execute all writers */
    for (int i = 0; i < mWriterCount; ++i) {
        Instrument* inst = mWriters[i];

        /* Set up instrument for this chunk */
        inst->setchunk(mBufsamps);
        inst->set_output_offset(0);

#ifdef IBUG
        printf("Tier %d: processing inst %p [%s]\n",
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
        printf("Tier %d: waiting for %d instrument tasks\n",
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

#ifdef TBUG
    printf("Tier %d: writePosition now %d, framesProduced=%lld\n",
           mBusID, mWritePosition, mFramesProduced);
#endif

#ifdef WBUG
    printf("EXITING Tier::runWriterCycle() for bus %d\n", mBusID);
#endif
}


/* ---------------------------------------------------- Tier::clearRegion --- */

void Tier::clearRegion(int startFrame, int numFrames)
{
    /* Handle wrap-around */
    int endFrame = startFrame + numFrames;

    if (endFrame <= mBufferSize) {
        /* No wrap - single clear */
        memset(&mRingBuffer[startFrame * mNumChannels], 0,
               sizeof(BUFTYPE) * numFrames * mNumChannels);
    } else {
        /* Wrap around - two clears */
        int firstPart = mBufferSize - startFrame;
        int secondPart = numFrames - firstPart;

        memset(&mRingBuffer[startFrame * mNumChannels], 0,
               sizeof(BUFTYPE) * firstPart * mNumChannels);
        memset(&mRingBuffer[0], 0,
               sizeof(BUFTYPE) * secondPart * mNumChannels);
    }
}


/* ------------------------------------------------- Tier::copyToConsumer --- */

void Tier::copyToConsumer(BufPtr dest, int readPos, int numFrames)
{
    /* Handle wrap-around */
    int endPos = readPos + numFrames;

    if (endPos <= mBufferSize) {
        /* No wrap - single copy */
        memcpy(dest, &mRingBuffer[readPos * mNumChannels],
               sizeof(BUFTYPE) * numFrames * mNumChannels);
    } else {
        /* Wrap around - two copies */
        int firstPart = mBufferSize - readPos;
        int secondPart = numFrames - firstPart;

        memcpy(dest, &mRingBuffer[readPos * mNumChannels],
               sizeof(BUFTYPE) * firstPart * mNumChannels);
        memcpy(&dest[firstPart * mNumChannels], &mRingBuffer[0],
               sizeof(BUFTYPE) * secondPart * mNumChannels);
    }
}


/* ---------------------------------------------------- Tier::growWriters --- */

void Tier::growWriters()
{
    int newCapacity = mWriterCapacity * 2;
    Instrument** newArray = new Instrument*[newCapacity];

    for (int i = 0; i < mWriterCount; ++i) {
        newArray[i] = mWriters[i];
    }

    delete[] mWriters;
    mWriters = newArray;
    mWriterCapacity = newCapacity;

#ifdef TBUG
    printf("Tier %d: grew writers array to capacity %d\n",
           mBusID, mWriterCapacity);
#endif
}


/* ------------------------------------------------ Tier::getWriterCount --- */

int Tier::getWriterCount() const
{
    return mWriterCount;
}


/* ---------------------------------------------- Tier::getConsumerCount --- */

int Tier::getConsumerCount() const
{
    return (int)mConsumers.size();
}