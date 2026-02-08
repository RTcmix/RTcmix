/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/*
 * Tier.h - Tier-based pull model for elastic audio bus buffering
 *
 * A Tier wraps a bus with a ring buffer, enabling instruments with
 * different input/output frame ratios to work correctly in chains.
 *
 * Key invariants:
 * - Output: Every writer produces exactly framesToRun() frames per cycle
 * - Input: Any consumer may request arbitrary frame counts
 * - The ring buffer absorbs the mismatch
 *
 * Threading model (MULTI_THREAD):
 * - All writers execute in parallel via TaskManager
 * - Synchronization via waitForTasks()
 * - Accumulation merged via mixToBus()
 *
 * Author: Claude Code / RTcmix Development Team
 */

#ifndef _TIER_H_
#define _TIER_H_

#include <rt_types.h>
#include <bus.h>

#ifdef MULTI_THREAD
#include <vector>
#endif

class Instrument;
class TaskManager;

/* Debug macro for tier operations - define TBUG to enable */
#undef TBUG

/* Default ring buffer size multiplier (relative to RTBUFSAMPS) */
#define TIER_BUFFER_MULTIPLIER 4

/* Special consumer ID for hardware output */
#define TIER_HARDWARE_CONSUMER -1


class Tier {
public:
    /**
     * Create a new tier for the specified bus.
     *
     * @param busID         The bus number this tier manages
     * @param numChannels   Number of audio channels for this bus
     * @param bufferSize    Size of ring buffer in frames (0 = auto-calculate)
     * @param bufsamps      The RTBUFSAMPS value (frames per chunk)
     */
    Tier(int busID, int numChannels, int bufferSize, int bufsamps);

    ~Tier();

    /**
     * Request frames from this tier.
     * Triggers production cycles as needed to satisfy the request.
     *
     * @param consumerID      ID returned by addConsumer()
     * @param requestedFrames Number of frames requested
     * @param dest            Destination buffer (interleaved)
     * @return                Number of frames actually delivered
     */
    int pullFrames(int consumerID, int requestedFrames, BufPtr dest);

    /**
     * Register a consumer that will read from this tier.
     * Each consumer gets an independent read cursor.
     *
     * @return Consumer ID to use with pullFrames()
     */
    int addConsumer();

    /**
     * Register an instrument that writes to this tier.
     *
     * @param inst  The instrument that produces audio for this tier
     */
    void addWriter(Instrument* inst);

    /**
     * Remove a writer from this tier.
     * Called when an instrument finishes.
     *
     * @param inst  The instrument to remove
     */
    void removeWriter(Instrument* inst);

    /**
     * Clear all state for a new audio run.
     */
    void reset();

    /* Accessors */
    int getBusID() const { return mBusID; }
    int getNumChannels() const { return mNumChannels; }
    int getWritePosition() const { return mWritePosition; }
    FRAMETYPE getFramesProduced() const { return mFramesProduced; }
    int getWriterCount() const;
    int getConsumerCount() const;
    int getBufsamps() const { return mBufsamps; }

    /**
     * Get frames available for a specific consumer.
     *
     * @param consumerID  The consumer ID
     * @return            Number of frames available to read
     */
    int framesAvailable(int consumerID) const;

#ifdef MULTI_THREAD
    /**
     * Set the TaskManager for parallel writer execution.
     *
     * @param tm  Pointer to the TaskManager
     */
    void setTaskManager(TaskManager* tm) { mTaskManager = tm; }
#endif

private:
    /* Bus identification */
    int mBusID;
    int mNumChannels;
    int mBufsamps;      /* frames per production chunk */

    /* Ring buffer */
    BufPtr mRingBuffer;
    int mBufferSize;          /* total size in frames */
    int mWritePosition;       /* current write position (frame offset) */
    FRAMETYPE mFramesProduced; /* total frames produced since reset */

    /* Writers (instruments that produce to this tier) */
    Instrument** mWriters;
    int mWriterCount;
    int mWriterCapacity;

    /* Readers (downstream consumers) */
    int* mReadCursors;        /* per-consumer read position */
    FRAMETYPE* mFramesConsumed; /* per-consumer total consumed */
    int mConsumerCount;
    int mConsumerCapacity;

    /**
     * Run one production cycle.
     * In MULTI_THREAD mode: adds tasks, waits, mixes.
     * In single-threaded mode: executes writers sequentially.
     */
    void runWriterCycle();

    /**
     * Clear a region of the ring buffer before writing.
     *
     * @param startFrame  Start position in frames
     * @param numFrames   Number of frames to clear
     */
    void clearRegion(int startFrame, int numFrames);

    /**
     * Copy frames from ring buffer to consumer's destination buffer.
     * Handles wrap-around at ring buffer end.
     *
     * @param dest       Destination buffer
     * @param readPos    Read position in ring buffer (frames)
     * @param numFrames  Number of frames to copy
     */
    void copyToConsumer(BufPtr dest, int readPos, int numFrames);

    /**
     * Grow the writers array if needed.
     */
    void growWriters();

    /**
     * Grow the consumers array if needed.
     */
    void growConsumers();

#ifdef MULTI_THREAD
    TaskManager* mTaskManager;
    std::vector<Instrument*> mActiveWriters;  /* temp for current cycle */
#endif

    /* Prevent copying */
    Tier(const Tier&);
    Tier& operator=(const Tier&);
};

#endif /* _TIER_H_ */