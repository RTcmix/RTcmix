/* RTcmix  - Copyright (C) 2000  The RTcmix Development Team
   See ``AUTHORS'' for a list of contributors. See ``LICENSE'' for
   the license to this software and for a DISCLAIMER OF ALL WARRANTIES.
*/

/*
 * InstrumentBus.h - Pull model for elastic audio bus buffering
 *
 * An InstrumentBus wraps a bus with a ring buffer, enabling instruments with
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

#ifndef _INSTRUMENTBUS_H_
#define _INSTRUMENTBUS_H_

#include <rt_types.h>
#include <bus.h>

#include <map>
#include <vector>

class Instrument;
class TaskManager;

/* Debug macro for InstrumentBus operations - define IBUG to enable */
#undef IBUG

/* Default ring buffer size multiplier (relative to RTBUFSAMPS) */
#define INSTBUS_BUFFER_MULTIPLIER 4



class InstrumentBus {
public:
    /**
     * Create a new InstrumentBus for the specified bus.
     *
     * @param busID         The bus number this InstrumentBus manages
     * @param numChannels   Number of audio channels for this bus
     * @param bufferSize    Size of ring buffer in frames (0 = auto-calculate)
     * @param bufsamps      The RTBUFSAMPS value (frames per chunk)
     */
    InstrumentBus(int busID, int numChannels, int bufferSize, int bufsamps);

    ~InstrumentBus();

    /**
     * Request frames from this InstrumentBus.
     * Triggers production cycles as needed to satisfy the request.
     *
     * @param consumer        The instrument requesting frames
     * @param requestedFrames Number of frames requested
     * @param dest            Destination buffer (interleaved)
     * @return                Number of frames actually delivered
     */
    int pullFrames(Instrument* consumer, int requestedFrames, BufPtr dest);

    /**
     * Register a consumer that will read from this InstrumentBus.
     * Each consumer gets an independent read cursor.
     *
     * @param inst  The instrument that reads from this InstrumentBus
     */
    void addConsumer(Instrument* inst);

    /**
     * Register an instrument that writes to this InstrumentBus.
     *
     * @param inst  The instrument that produces audio for this InstrumentBus
     */
    void addWriter(Instrument* inst);

    /**
     * Remove a writer from this InstrumentBus.
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
     * @param consumer  The instrument requesting availability info
     * @return          Number of frames available to read
     */
    int framesAvailable(Instrument* consumer) const;

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

    /* Writers (instruments that produce to this InstrumentBus) */
    std::vector<Instrument*> mWriters;

    /* Readers (downstream consumers) - keyed by Instrument pointer */
    struct ConsumerState {
        int readCursor;           /* position in ring buffer (frames) */
        FRAMETYPE framesConsumed; /* total frames consumed */
        ConsumerState() : readCursor(0), framesConsumed(0) {}
    };
    std::map<Instrument*, ConsumerState> mConsumers;

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

#ifdef MULTI_THREAD
    TaskManager* mTaskManager;
    std::vector<Instrument*> mActiveWriters;  /* temp for current cycle */
#endif

    /* Prevent copying */
    InstrumentBus(const InstrumentBus&);
    InstrumentBus& operator=(const InstrumentBus&);
};

#endif /* _INSTRUMENTBUS_H_ */