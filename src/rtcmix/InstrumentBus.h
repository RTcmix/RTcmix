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
 * Production model:
 * - pullFrames() triggers demand-driven production by popping instruments
 *   from rtQueue, computing timing, executing, and re-queuing
 * - Processes both TO_AUX and AUX_TO_AUX queues for this bus
 * - All external state accessed via RTcmix statics (rtQueue, TaskManager)
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
#include "Lockable.h"

class Instrument;

/* Define INSTBUS_PERSIST_DATA to enable cross-cycle data persistence.
 * When defined: aux_buffer is larger, data persists across inTraverse cycles.
 * When undefined: aux_buffer is normal size, everything balances per cycle.
 */
#undef INSTBUS_PERSIST_DATA

#ifdef INSTBUS_PERSIST_DATA
#define INSTBUS_BUFFER_MULTIPLIER 4
#else
#define INSTBUS_BUFFER_MULTIPLIER 1
#endif


class InstrumentBus {
public:
    /**
     * Create a new InstrumentBus for the specified bus.
     * The InstrumentBus uses the global aux_buffer directly as its ring buffer.
     *
     * @param busID         The bus number this InstrumentBus manages
     * @param bufsamps      The RTBUFSAMPS value (frames per chunk)
     */
    InstrumentBus(int busID, int bufsamps);

    ~InstrumentBus();

    /**
     * Request frames from this InstrumentBus.
     * Runs production cycles (popping from rtQueue) until enough frames
     * are available, then advances consumer's read cursor.
     *
     * After calling this, the caller should read directly from
     * aux_buffer[busID] starting at the returned position.
     *
     * @param consumer        The instrument requesting frames
     * @param requestedFrames Number of frames requested
     * @return                Read position in aux_buffer (frame offset)
     */
    int pullFrames(Instrument* consumer, int requestedFrames);

    /**
     * Register a consumer that will read from this InstrumentBus.
     * Each consumer gets an independent read cursor.
     *
     * @param inst  The instrument that reads from this InstrumentBus
     */
    void addConsumer(Instrument* inst);

    /**
     * Clear all state for a new audio run.
     */
    void reset();

    /* Accessors */
    int getBusID() const { return mBusID; }
    int getBufferSize() const { return mBufferSize; }
    FRAMETYPE getFramesProduced() const { return mFramesProduced; }
    int getConsumerCount() const;
    int getBufsamps() const { return mBufsamps; }

    /** Current write region start (derived from mFramesProduced) */
    int getWriteRegionStart() const { return (int)(mFramesProduced % mBufferSize); }

    /**
     * Get frames available for a specific consumer.
     *
     * @param consumer  The instrument requesting availability info
     * @return          Number of frames available to read
     */
    int framesAvailable(Instrument* consumer) const;

    /**
     * Get read position for consumer and advance cursor.
     *
     * @param consumer  The instrument reading
     * @param frames    Number of frames being read
     * @return          Read position in aux_buffer (frame offset)
     */
    int getReadPosition(Instrument* consumer, int frames);

private:
    /* Bus identification */
    int mBusID;
    int mBufsamps;      /* frames per production chunk */

    /* Ring buffer state (uses aux_buffer directly, no separate allocation) */
    int mBufferSize;          /* total size in frames (INSTBUS_BUFFER_MULTIPLIER * bufsamps) */
    FRAMETYPE mFramesProduced; /* total frames produced since reset */

    /* Readers (downstream consumers) - keyed by Instrument pointer */
    struct ConsumerState {
        FRAMETYPE framesConsumed; /* total frames consumed */
        ConsumerState() : framesConsumed(0) {}
    };
    std::map<Instrument*, ConsumerState> mConsumers;

    /**
     * Run one production cycle by popping instruments from rtQueue,
     * computing timing, executing, and re-queuing.
     * Processes both TO_AUX and AUX_TO_AUX queues for this bus.
     *
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

    /* Per-bus production lock: serializes pullFrames/runWriterCycle.
     * In the push model, bus production was inherently serial (phase ordering).
     * This lock restores that invariant for pull-based production. */
    Lockable mPullLock;

    /* Prevent copying */
    InstrumentBus(const InstrumentBus&);
    InstrumentBus& operator=(const InstrumentBus&);
};

#endif /* _INSTRUMENTBUS_H_ */
