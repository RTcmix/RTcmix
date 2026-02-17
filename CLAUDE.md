# RTcmix Bus System: InstrumentBus-Based Pull Model Conversion

## Project Overview

This document describes the plan to convert RTcmix's audio bus routing system from a push model to an InstrumentBus-based pull model. The goal is to enable instruments with different input/output frame ratios (e.g., interpolators, decimators, time-stretchers) to work correctly in instrument chains.

## The Problem with the Current Push Model

The current system pushes a fixed frame count (RTBUFSAMPS) through the entire instrument chain:

```
WAVETABLE → 1024 frames → TRANS (2:1 interpolator) → ??? → output needs 1024

Problem: TRANS receives 1024 input frames but can only produce 512 output frames.
         The output stage starves.
```

The output stage MUST receive exactly RTBUFSAMPS frames or it will starve/overflow. But pushing fixed frame counts forces all intermediate instruments to have input==output, which isn't true for rate-changing instruments.

## The Solution: InstrumentBus-Based Pull Model

### Core Concept: InstrumentBus

An **InstrumentBus** wraps a bus with ring buffer management:
- A bus (e.g., aux0)
- The instruments that write to it
- Uses aux_buffer directly as ring buffer (with configurable multiplier for persistence)
- Read cursors (one per downstream consumer, tracked by Instrument pointer)
- Write tracking (for accumulating multiple writers)

The InstrumentBus uses the **same threading model** as the current per-bus execution:
- All writers run in parallel via TaskManager
- Synchronization via `waitForTasks()`
- Accumulation merged via `mixToBus()`

### Key Invariants

1. **Output guarantee**: Every instrument writes exactly `framesToRun()` frames per run
2. **Input flexibility**: Any instrument may request more or less than `framesToRun()` frames from its upstream tier

This asymmetry is the key insight:
- Production: fixed-size chunks (`framesToRun()` per instrument per run)
- Consumption: variable-size requests from downstream
- The InstrumentBus ring buffer absorbs the mismatch

### How It Works

```
True Pull:
  output requests 1024 frames from output InstrumentBus
       ↓
  TRANS (2:1): "To output 1024, I need 2048 input frames"
       ↓
  aux0 InstrumentBus: runs WAVETABLE twice (2 × 1024 = 2048 frames)
       ↓
  TRANS: reads 2048 from aux0 InstrumentBus, outputs 1024
       ↓
  output receives 1024 frames ✓
```

## Current Architecture Reference

### Key Files

| File | Purpose | Lines |
|------|---------|-------|
| `src/rtcmix/bus_config.cpp` | Bus configuration, play order, addToBus(), mixToBus() | ~1015 |
| `src/rtcmix/intraverse.cpp` | Main audio loop, scheduling, task management | ~800 |
| `src/rtcmix/Instrument.cpp` | Base instrument class, exec/run/addout | ~503 |
| `src/rtcmix/buffers.cpp` | Buffer allocation and clearing | ~232 |
| `src/rtcmix/rtgetin.cpp` | Input reading (already pull-based) | ~278 |
| `src/rtcmix/TaskManager.h` | Thread pool task management | ~144 |

### Current Threading Model (MULTI_THREAD)

The current system processes each bus with this pattern (`intraverse.cpp:424-551`):

```cpp
// For each bus in current phase (TO_AUX, AUX_TO_AUX, TO_OUT):
vector<Instrument*> instruments;

// 1. Collect all instruments writing to this bus
while (rtQSize > 0 && rtQchunkStart < bufEndSamp) {
    Iptr = rtQueue[busq].pop(&rtQchunkStart);
    // ... set up chunksamps, offset, etc. ...

    instruments.push_back(Iptr);
    taskManager->addTask<Instrument, int, BusType, int, &Instrument::exec>
        (Iptr, bus_type, bus);
}

// 2. Synchronization point - wait for all writers to complete
taskManager->waitForTasks(instruments);

// 3. Merge per-thread accumulation vectors into bus buffer
RTcmix::mixToBus();

// 4. Re-queue or unref instruments
for (Iptr : instruments) {
    if (endsamp > bufEndSamp)
        rtQueue[busq].pushUnsorted(Iptr, rtQchunkStart + chunksamps);
    else
        Iptr->unref();
}
instruments.clear();
```

### Thread-Safe Accumulation

In `MULTI_THREAD` mode, `addToBus()` queues mix operations per-thread (`bus_config.cpp:637-648`):

```cpp
void RTcmix::addToBus(BusType type, int bus, BufPtr src, int offset, int endfr, int chans)
{
    // Each thread accumulates to its own vector - no locking needed
    mixVectors[RTThread::GetIndexForThread()].push_back(
        MixData(src, dest, endfr - offset, chans)
    );
}
```

Then `mixToBus()` merges all thread-local vectors (`bus_config.cpp:676-684`):

```cpp
void RTcmix::mixToBus()
{
    for (int i = 0; i < RT_THREAD_COUNT; ++i) {
        std::vector<MixData> &vector = mixVectors[i];
        std::for_each(vector.begin(), vector.end(), mixOperation);
        vector.clear();
    }
}
```

### Existing Instrument Patterns

Analysis of `insts/` directory confirms:
- **All instruments** write exactly `framesToRun()` output frames
- **Input consumption varies** based on processing requirements (TRANS, PVOC, LPCIN, etc.)

| Instrument | Input Pattern | Output Pattern |
|------------|---------------|----------------|
| TRANS | Variable (interpolation ratio) | Fixed `framesToRun()` |
| PVOC | `_decimation` frames per hop | Fixed `framesToRun()` |
| LPCIN | Variable `_counter` (pitch-dependent) | Fixed `framesToRun()` |
| WAVETABLE | None (generator) | Fixed `framesToRun()` |
| MIX | Fixed `framesToRun()` | Fixed `framesToRun()` |

## InstrumentBus Architecture

### Structure

```
┌─────────────────────────────────────────────────────────┐
│               INSTRUMENTBUS (e.g., aux0)                │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │     aux_buffer (used as ring buffer)             │   │
│  │  [frame0][frame1][frame2]...[frameN]            │   │
│  │      ↑                          ↑                │   │
│  │   read cursors              write cursor         │   │
│  │   (per consumer Instrument) (fixed chunks)       │   │
│  └─────────────────────────────────────────────────┘   │
│                         ↑                               │
│           ┌─────────────┴─────────────┐                │
│           │                           │                 │
│     [Instrument A]             [Instrument B]          │
│     (writes framesToRun())    (writes framesToRun())   │
│           │                           │                 │
│           └─────────────┬─────────────┘                │
│                         │                               │
│              TaskManager parallel execution             │
│              waitForTasks() synchronization             │
│              mixToBus() accumulation                    │
│                         │                               │
│                  write cursor advances                  │
│                  by framesToRun() per cycle             │
└─────────────────────────────────────────────────────────┘
                          │
          variable requests from downstream
                          ↓
              ┌───────────┴───────────┐
              │                       │
        [Consumer X]            [Consumer Y]
        (requests M frames)     (requests N frames)
```

### Ring Buffer Management

**Buffer allocation:**
- InstrumentBus uses aux_buffer directly (no separate allocation)
- Buffer size controlled by `INSTBUS_BUFFER_MULTIPLIER` (1 for non-persistent, 4 for persistent mode)
- Configured via `INSTBUS_PERSIST_DATA` define

**Write side (fixed chunks, parallel execution):**
- Clear the next region of aux_buffer
- Add tasks for ALL writer instruments to TaskManager
- `waitForTasks()` - block until all complete
- `mixToBus()` - merge per-thread accumulations into aux_buffer
- Advance write cursor by `framesToRun()`

**Read side (variable requests):**
- Each consumer tracked by Instrument pointer in `std::map<Instrument*, ConsumerState>`
- Consumers request arbitrary frame counts via `pullFrames()`
- `pullFrames()` returns read position; caller reads directly from aux_buffer
- Read cursors advance independently

**Safe to overwrite:**
```
oldest_read = min(all consumer read cursors)
can_overwrite_up_to = oldest_read
```

### Timeline Alignment

For instruments reading from aux buses (not file input):
- All consumers are time-synchronized
- Frame N in the InstrumentBus buffer represents the same moment for all readers
- Read cursors may diverge due to different consumption rates
- But they're reading from the same timeline

## Build Configuration Requirements

### Dual Compilation: MULTI_THREAD and Single-Threaded

The implementation must support both build modes, matching the current system:

```cpp
#ifdef MULTI_THREAD
#include "TaskManager.h"
#include <vector>
#endif
```

**MULTI_THREAD mode:**
- Uses TaskManager for parallel instrument execution
- Per-thread accumulation via `mixVectors[]`
- `waitForTasks()` synchronization
- `mixToBus()` to merge thread-local vectors

**Single-threaded mode:**
- Direct sequential instrument execution
- Direct accumulation into tier buffer (no mixVectors)
- No TaskManager dependency

### Debug Logging Macros

Match the existing debug macro pattern from `intraverse.cpp`:

```cpp
#undef ALLBUG    // All debug output
#undef BBUG      // Bus debugging (verbose!)
#undef DBUG      // General debug
#undef WBUG      // "Where we are" prints
#undef IBUG      // Instrument debugging
#undef BBUG      // Verbose bus debugging
```

All new code must include equivalent debug logging:

```cpp
#ifdef IBUG
    printf("InstrumentBus::pullFrames(consumer=%p, requested=%d) entering\n",
           consumer, requestedFrames);
    printf("InstBus: adding inst %p to taskmgr [%s]\n", inst, inst->name());
#endif

#ifdef WBUG
    printf("ENTERING InstrumentBus::runWriterCycle()\n");
#endif
```

Note: IBUG is used for both Instrument and InstrumentBus debugging.

## Implementation Plan

### Phase 1: InstrumentBus Class

Create new `InstrumentBus` class that wraps a bus with ring buffer management.
Must support both MULTI_THREAD and single-threaded builds:

```cpp
class InstrumentBus {
public:
    InstrumentBus(int busID, int bufsamps);
    ~InstrumentBus();

    // Called by downstream instruments to request input
    // Returns read position in aux_buffer; caller reads directly
    int pullFrames(Instrument* consumer, int requestedFrames);

    // Register a consumer (tracked by Instrument pointer)
    void addConsumer(Instrument* inst);

    // Register/remove a writer instrument
    void addWriter(Instrument* inst);
    void removeWriter(Instrument* inst);

    // State management
    void reset();
    void resetWritePosition();

    // Accessors for debugging
    int getBusID() const { return mBusID; }
    int getWritePosition() const { return mWritePosition; }
    FRAMETYPE getFramesProduced() const { return mFramesProduced; }
    int framesAvailable(Instrument* consumer) const;

private:
    int mBusID;
    int mBufsamps;           // Frames per production chunk

    // Ring buffer state (uses aux_buffer directly)
    int mBufferSize;         // Total size (INSTBUS_BUFFER_MULTIPLIER * bufsamps)
    int mWritePosition;      // Current write position
    FRAMETYPE mFramesProduced; // Total frames produced

    // Writers (instruments that produce to this InstrumentBus)
    std::vector<Instrument*> mWriters;

    // Readers - tracked by Instrument pointer
    struct ConsumerState {
        int readCursor;
        FRAMETYPE framesConsumed;
    };
    std::map<Instrument*, ConsumerState> mConsumers;

    // Run one production cycle (handles both threaded and non-threaded)
    void runWriterCycle();

    // Ring buffer utilities
    void clearRegion(int startFrame, int numFrames);
    void copyToConsumer(BufPtr dest, int readPos, int numFrames);

#ifdef MULTI_THREAD
    TaskManager* mTaskManager;
    std::vector<Instrument*> mActiveWriters;
#endif
};
```

### Phase 2: InstrumentBus Production Cycle

The InstrumentBus writer cycle mirrors the current per-bus pattern, with both MULTI_THREAD and single-threaded paths:

```cpp
void InstrumentBus::runWriterCycle() {
    // Matches current intraverse.cpp pattern for a single bus

#ifdef WBUG
    printf("ENTERING InstrumentBus::runWriterCycle() for bus %d\n", mBusID);
#endif

    // 1. Clear the aux_buffer region we're about to write
    clearRegion(mWritePosition, mBufsamps);

#ifdef BBUG
    printf("InstBus %d: cleared region [%d, %d)\n",
           mBusID, mWritePosition, mWritePosition + mBufsamps);
#endif

#ifdef MULTI_THREAD
    mActiveWriters.clear();
#endif

    // 2. Execute all writers
    for (size_t i = 0; i < mWriters.size(); ++i) {
        Instrument* inst = mWriters[i];
        // Set up instrument for this chunk
        inst->setchunk(mBufsamps);
        inst->set_output_offset(mWritePosition);

#ifdef IBUG
        printf("InstBus %d: processing inst %p [%s]\n", mBusID, inst, inst->name());
#endif

#ifdef MULTI_THREAD
        mActiveWriters.push_back(inst);
        mTaskManager->addTask<Instrument, int, BusType, int, &Instrument::exec>
            (inst, BUS_AUX_OUT, mBusID);
#else
        // Single-threaded: execute directly
        inst->exec(BUS_AUX_OUT, mBusID);
#endif
    }

#ifdef MULTI_THREAD
    // 3. Wait for all writers to complete
    if (!mActiveWriters.empty()) {
#ifdef DBUG
        printf("InstBus %d: waiting for %d instrument tasks\n",
               mBusID, (int)mActiveWriters.size());
#endif
        mTaskManager->waitForTasks(mActiveWriters);

        // 4. Merge per-thread accumulations into aux_buffer
        RTcmix::mixToBus();
    }
#endif

    // 5. Advance write position
    mWritePosition = (mWritePosition + mBufsamps) % mBufferSize;
    mFramesProduced += mBufsamps;

#ifdef BBUG
    printf("InstBus %d: writePosition now %d, framesProduced=%lld\n",
           mBusID, mWritePosition, mFramesProduced);
#endif

    // NOTE: Instrument lifecycle (re-queue/unref) is an open design question.
    // Currently handled separately from runWriterCycle.

#ifdef WBUG
    printf("EXITING InstrumentBus::runWriterCycle() for bus %d\n", mBusID);
#endif
}
```

### Phase 3: InstrumentBus Pull Interface

```cpp
int InstrumentBus::pullFrames(Instrument* consumer, int requestedFrames) {
#ifdef WBUG
    printf("ENTERING InstrumentBus::pullFrames(bus=%d, consumer=%p, frames=%d)\n",
           mBusID, consumer, requestedFrames);
#endif

    std::map<Instrument*, ConsumerState>::iterator it = mConsumers.find(consumer);
    assert(it != mConsumers.end());

    ConsumerState& state = it->second;
    int readPos = state.readCursor;

#ifdef BBUG
    printf("InstBus %d: consumer %p requesting %d frames, available=%d\n",
           mBusID, consumer, requestedFrames, framesAvailable(consumer));
#endif

    // Loop until we have enough frames
    while (framesAvailable(consumer) < requestedFrames) {
        if (mWriters.empty()) {
            // No writers - aux_buffer should be zeros
            break;
        }
        runWriterCycle();  // Runs writers, waits, mixes, advances
    }

    // Advance this consumer's read cursor
    state.readCursor = (state.readCursor + requestedFrames) % mBufferSize;
    state.framesConsumed += requestedFrames;

#ifdef BBUG
    printf("InstBus %d: consumer %p readCursor %d -> %d\n",
           mBusID, consumer, readPos, state.readCursor);
#endif

#ifdef WBUG
    printf("EXITING InstrumentBus::pullFrames(bus=%d) returning readPos=%d\n",
           mBusID, readPos);
#endif

    // Return read position; caller reads directly from aux_buffer
    return readPos;
}
```

### Phase 4: Integrate with Instrument Input

Modify `rtgetin()` to pull from InstrumentBus:

```cpp
int Instrument::rtgetin(float* inarr, int nsamps) {
    const int inchans = inputChannels();
    const int frames = nsamps / inchans;

    // Check if InstrumentBus pull model is active for this bus
    InstrumentBusManager* mgr = RTcmix::getInstBusManager();
    InstrumentBus* instBus = mgr->getInstBus(auxin[0]);

    if (instBus != NULL) {
        // Pull model: trigger production and get read position
        for (int n = 0; n < auxin_count; n++) {
            int chan = auxin[n];
            instBus = mgr->getInstBus(chan);
            int readPos = instBus->pullFrames(this, frames);
        }
        // Read from aux_buffers and interleave into inarr
        RTcmix::readFromAuxBus(inarr, inchans, frames, auxin, auxin_count, readPos);
        return nsamps;
    }

    // Fall back to existing file/device/legacy aux input
    // ... existing rtgetin logic ...
}
```

### Phase 5: Entry Point (inTraverse)

The main loop initiates a pull from the output InstrumentBus:

```cpp
bool RTcmix::inTraverse(AudioDevice* device, void* arg) {
    // ... existing setup, heap popping, buffer management ...

    // Process all queues for SETUP only (chunk info, lifecycle tracking)
    // but do NOT call exec() directly - instruments run via pull
    // (See "intraverse Changes" section below for details)

    // Pull RTBUFSAMPS frames from output InstrumentBus
    // This triggers cascading pulls through all upstream InstrumentBuses
    outputInstBus->pullFrames(HARDWARE_CONSUMER, RTBUFSAMPS);

    // Send to hardware (unchanged)
    rtsendsamps(device);

    // Handle instrument lifecycle (re-queue or unref)
    // (Open design question: where exactly this happens)

    return playEm;
}
```

### intraverse Changes

The key change to intraverse is separating **setup** from **execution**:

**What intraverse continues to do:**
- Buffer cycle management (bufStartSamp, bufEndSamp, frameCount)
- Queue management (pop instruments, track timing)
- Chunk setup (ichunkstart, output_offset, chunksamps)
- Initiate pull from output InstrumentBus
- Handle instrument lifecycle (re-queue/unref) - open design question

**What intraverse no longer does:**
- Direct exec() calls for any instruments
- The TO_AUX → AUX_TO_AUX → TO_OUT execution order is replaced by pull-based execution

All instruments are triggered by the pull chain starting from output.

### Phase 6: InstrumentBus Graph Construction

Registration happens in `Instrument::set_bus_config()`:

```cpp
void Instrument::set_bus_config(const char* inst_name) {
    // ... existing bus slot setup ...

    // Register with InstrumentBus system for pull-based audio routing
    InstrumentBusManager* instBusMgr = RTcmix::getInstBusManager();

    // Register as writer to all auxout buses
    for (int i = 0; i < _busSlot->auxout_count; i++) {
        instBusMgr->addWriter(_busSlot->auxout[i], this);
    }

    // Register as consumer of all auxin buses
    for (int i = 0; i < _busSlot->auxin_count; i++) {
        instBusMgr->addConsumer(_busSlot->auxin[i], this);
    }
}
```

InstrumentBusManager creates InstrumentBus objects on demand:

```cpp
InstrumentBus* InstrumentBusManager::getOrCreateInstBus(int busID) {
    if (mInstBuses[busID] == NULL) {
        // aux_buffer already allocated by bus_config.cpp
        mInstBuses[busID] = new InstrumentBus(busID, mBufsamps);
        // ... TaskManager setup for MULTI_THREAD ...
    }
    return mInstBuses[busID];
}
```

## Example: WAVETABLE → TRANS → Output

```
Configuration:
  bus_config("WAVETABLE", "aux 0 out")       → writes to aux0 InstrumentBus
  bus_config("TRANS", "aux 0 in", "aux 1 out") → reads aux0, writes aux1 InstrumentBus
  bus_config("STEREO", "aux 1 in", "out 0-1")  → reads aux1, writes output InstrumentBus

Runtime (hardware requests 512 frames):

  1. inTraverse() calls outputInstBus->pullFrames(512)

  2. outputInstBus needs STEREO to produce 512 frames
     - Runs STEREO via runWriterCycle()
     - STEREO::run() calls rtgetin() requesting 512 frames

  3. rtgetin() calls aux1InstBus->pullFrames(this, 512)

  4. aux1InstBus needs TRANS to produce 512 frames
     - Runs TRANS via runWriterCycle()
     - TRANS::run() calls rtgetin() requesting 1024 frames (2:1 ratio)

  5. rtgetin() calls aux0InstBus->pullFrames(this, 1024)

  6. aux0InstBus needs 1024 frames, has 0 available
     Loop iteration 1:
       - clearRegion(0, 512)
       - addTask(WAVETABLE)
       - waitForTasks()
       - mixToBus()
       - mWritePosition = 512, mFramesProduced = 512

     Loop iteration 2:
       - clearRegion(512, 512)
       - addTask(WAVETABLE)
       - waitForTasks()
       - mixToBus()
       - mWritePosition = 1024, mFramesProduced = 1024

     Now 1024 frames available

  7. pullFrames returns readPos=0; TRANS reads 1024 frames from aux_buffer[0]

  8. TRANS processes 1024 input → 512 output frames to aux_buffer[1]

  9. pullFrames returns readPos=0; STEREO reads 512 frames from aux_buffer[1]

  10. STEREO writes to out_buffer

  11. rtsendsamps() sends to hardware
```

## What Changes vs. Current System

| Aspect | Current (Push) | InstrumentBus Model (Pull) |
|--------|----------------|----------------------------|
| Trigger | Play list order | Downstream request |
| Frame count | Fixed RTBUFSAMPS everywhere | Variable requests, fixed production |
| Buffer type | Simple arrays, cleared each cycle | aux_buffer as ring buffer (configurable persistence) |
| Threading | Per-bus: addTask→wait→mix | Per-InstrumentBus: addTask→wait→mix (same) |
| Signal flow | Implicit in play lists | Explicit in pull chain |
| intraverse role | Setup + execution | Setup + initiate pull (execution via pull chain) |

## What Stays Unchanged

- `Instrument::run()` - generates samples into private `outbuf`
- `Instrument::exec()` - run + addout coordination
- `Instrument::rtaddout()` - writes to private `outbuf`
- `Instrument::addout()` - queues mix operation
- `addToBus()` - per-thread accumulation (MULTI_THREAD) or direct (single-threaded)
- `mixToBus()` - merge thread-local vectors (MULTI_THREAD only)
- `TaskManager` - parallel execution infrastructure (MULTI_THREAD only)
- `heap` scheduling system
- **Build configurations** - both MULTI_THREAD and single-threaded must work
- **Debug macro pattern** - DBUG, IBUG, WBUG, BBUG, ALLBUG

## Testing Strategy

### Build Configuration Tests
1. **MULTI_THREAD build** - verify compilation and execution
2. **Single-threaded build** - verify compilation and execution
3. **Debug builds** - verify all debug macros compile (DBUG, IBUG, WBUG, BBUG, TBUG, ALLBUG)
4. **EMBEDDED build** - verify compatibility

### Unit Tests
1. Single writer, single reader InstrumentBus
2. Multiple writers to same InstrumentBus (verify parallel exec + mixing)
3. Multiple readers from same InstrumentBus (verify independent cursors)
4. Request larger than single production cycle (verify looping)
5. Request smaller than production (verify buffering)

### Integration Tests
1. Simple chain: WAVETABLE → aux → MIX → out
2. Rate change: WAVETABLE → TRANS(2:1) → STEREO → out
3. Multiple sources: WAVETABLE + NOISE → aux → MIX → out
4. Deep chain: A → aux0 → B → aux1 → C → out

### Regression Tests
- All existing test scores in `test/suite/`
- Verify TRANS, PVOC work correctly in chains
- **Run all tests in both MULTI_THREAD and single-threaded builds**

## Buffer Sizing

InstrumentBus uses aux_buffer directly with configurable size multiplier:

```cpp
#undef INSTBUS_PERSIST_DATA  // Define for cross-cycle persistence

#ifdef INSTBUS_PERSIST_DATA
#define INSTBUS_BUFFER_MULTIPLIER 4   // Larger buffer for persistence
#else
#define INSTBUS_BUFFER_MULTIPLIER 1   // Standard size
#endif
```

- Non-persistent mode (MULTIPLIER=1): Buffer equals RTBUFSAMPS, balanced per cycle
- Persistent mode (MULTIPLIER=4): Larger buffer allows data to persist across cycles

The output InstrumentBus must produce exactly `RTBUFSAMPS` frames per pull (to match hardware).

## Implementation Status

### Completed

The following infrastructure has been implemented:

1. **InstrumentBus class** (`src/rtcmix/InstrumentBus.h`, `src/rtcmix/InstrumentBus.cpp`)
   - Uses aux_buffer directly as ring buffer (no separate allocation)
   - Per-consumer read cursors tracked by Instrument pointer (`std::map<Instrument*, ConsumerState>`)
   - Writer registration and removal
   - `pullFrames()` - triggers production cycles, returns read position
   - `runWriterCycle()` - executes writers in parallel (MULTI_THREAD) or sequentially
   - Full debug logging support (BBUG, WBUG, IBUG, DBUG)
   - `INSTBUS_PERSIST_DATA` / `INSTBUS_BUFFER_MULTIPLIER` for configurable persistence

2. **InstrumentBusManager class** (`src/rtcmix/InstrumentBusManager.h`, `src/rtcmix/InstrumentBusManager.cpp`)
   - Creates and manages InstrumentBus objects per aux bus
   - `getOrCreateInstBus()` - lazy creation
   - `addWriter()` / `addConsumer()` - registration
   - `removeWriter()` - cleanup
   - Integrates with TaskManager for parallel execution

3. **Instrument integration** (`src/rtcmix/Instrument.h`, `src/rtcmix/Instrument.cpp`)
   - Consumer registration in `set_bus_config()` for auxin buses
   - Writer registration **deferred** to intraverse heap-pop time (NOT in set_bus_config)
   - Cleanup in destructor (removeWriter)

4. **Input path modification** (`src/rtcmix/rtgetin.cpp`)
   - Checks for InstrumentBus via `getInstBus()`
   - Calls `pullFrames()` for each auxin channel
   - Uses `readFromAuxBus()` with returned read position
   - Falls back to legacy path if no InstrumentBus

5. **RTcmix integration** (`src/rtcmix/RTcmix.h`, `src/rtcmix/RTcmix.cpp`)
   - Static `instBusManager` member
   - `friend class InstrumentBus` for aux_buffer access
   - `getInstBusManager()` accessor

6. **TaskManager nested parallelism** (`src/rtcmix/TaskManager.cpp`, `src/rtcmix/TaskManager.h`)
   - WaitContext support for nested `startAndWait()` calls
   - Participatory waiting (TaskThreads help run tasks while waiting)
   - Non-TaskThread callers (main thread) block on semaphore

7. **intraverse.cpp modifications** (`src/rtcmix/intraverse.cpp`)
   - Writer registration added to heap-pop loop (after configure())
   - TO_AUX and AUX_TO_AUX instruments are **deferred** (not exec'd directly)
   - Only TO_OUT instruments are executed, triggering pull chain via rtgetin
   - Deferred instrument lifecycle (re-queue/unref) handled after TO_OUT completes
   - `deferredInsts` vector tracks deferred instruments with their busq for re-queuing

8. **Test scores** (`test/suite/instbus_*.sco`) - 17 test scores:
   - `instbus_basic_test.sco` - single WAVETABLE → aux → MIX → out
   - `instbus_chain_test.sco` - WAVETABLE → aux → MIX → aux → MIX → out
   - `instbus_multi_writer_test.sco` - 2 WAVETABLEs → same aux → MIX → out
   - `instbus_trans_test.sco` - WAVETABLE → TRANS → STEREO chain
   - `instbus_trans_down_test.sco` - transpose down test
   - `instbus_dual_mono_separate_test.sco` - 2 WAVETABLEs on separate aux buses
   - `instbus_dual_mono_to_stereo_test.sco` - 2 mono aux → stereo output
   - `instbus_multilevel_merge_test.sco` - 3-level cascade, 3 sources (PASS: ~12000)
   - `instbus_multi_writer_cascade_test.sco` - 2 writers/bus at 2 stages (PASS: ~12000)
   - `instbus_deep_chain_test.sco` - 5-stage MIX chain (PASS: ~11809)
   - `instbus_asymmetric_tree_test.sco` - 3 branches depth 3/2/1 (FAIL: 20000 vs expected 12000)
   - `instbus_staggered_entry_test.sco` - 3 writers staggered t=0,1,2 (FAIL: only ~4267)
   - `instbus_diamond_test.sco` - diamond routing topology
   - `instbus_parallel_merge_test.sco` - parallel paths merging
   - `instbus_stereo_merge_test.sco` - stereo merge routing
   - `instbus_quad_test.sco` - quad output routing
   - `instbus_complex_quad_test.sco` - complex quad routing

### Resolved Issues

1. **Double execution** (FIXED): intraverse now defers TO_AUX/AUX_TO_AUX instruments
   instead of executing them directly. Only TO_OUT instruments exec, triggering
   pulls through the InstrumentBus chain.

2. **Crash from unconfigured instruments** (FIXED): Writer registration moved from
   `set_bus_config()` (parse time) to intraverse heap-pop (runtime), so only
   instruments that have reached their start time are registered.

3. **STEREO pan issue** (FIXED): `instbus_dual_mono_to_stereo_test.sco` was using
   `STEREO(0, 0, dur, 1, 0.5)` with 2 input channels but only 1 pan pfield.
   Fixed to `STEREO(0, 0, dur, 1, 1.0, 0.0)`.

### Resolved: InstrumentBus Owns Queue-Based Production Loop

**Design resolved 2026-02-14. Full plan: `~/.claude/plans/eager-zooming-pancake.md`**

The autonomous `runWriterCycle()` (using mWriters list) is replaced with a
queue-based production loop inside `pullFrames()` that pops from rtQueue,
computes timing, execs, and re-queues — the same pattern as intraverse but
driven by demand.

**Key decisions:**
1. **No global bufEndSamp/bufStartSamp** — output InstrumentBus's `mFramesProduced`
   is the system master clock. Each aux bus has its own timeline.
2. **Ring buffer positions derived** — `mWritePosition` eliminated; use
   `mFramesProduced % mBufferSize`. Only absolute counters stored.
3. **Instruments re-enqueue/dequeue through rtQueue** — each production iteration
   is a full mini-buffer-cycle. rtQueue handles all timing edge cases.
4. **InstrumentBus owns pullFrames + production loop** — accesses rtQueue,
   TaskManager, mixToBus via RTcmix statics. No callback mechanism needed.
   Processes both TO_AUX and AUX_TO_AUX queues for its bus.
5. **Parameter timing for rate-changing chains** — known characteristic of new
   functionality. Real-time params (MIDI, OSC) upstream of rate changers update
   at output buffer rate, not scaled. No regression (TRANS/PVOC can't read aux
   buses in push model).
6. **Heap pop V1 limitation** — instruments starting beyond output ceiling on
   upstream buses may be delayed one buffer. Acceptable for V1.

### Known Test Failures

1. **instbus_asymmetric_tree_test**: Peak 20000 instead of expected 12000
   - 3 branches of depth 3/2/1 converge on aux 6
   - Likely: extra writers being registered or MIX instruments counted as writers multiple times

2. **instbus_staggered_entry_test**: Peak ~4267 instead of expected ~12000
   - 3 WAVETABLEs at t=0,1,2 writing to same aux bus
   - Only first writer contributes properly; `runWriterCycle` overrides per-instrument timing

3. **Channel 1 = 0 on MIX-to-output tests**: MIX with `MIX(0, 0, dur, 1, 0, 1)` outputs
   mono to ch0 only. May be expected behavior for mono aux → stereo out routing.

### Current Debug State

Debug output is **very verbose** (IBUG and DBUG #defined, not #undef'd):
- `InstrumentBus.cpp` - IBUG, DBUG defined (lines 27-28)
- `Instrument.cpp` - IBUG defined (line 29)
- `rtgetin.cpp` - IBUG defined (line 26)
- `InstrumentBusManager.cpp` - check for IBUG/DBUG defines

**Before running tests**: either #undef these or pipe output through grep for specific lines.
Tests will appear to hang if debug output is active and piped through `head`/`tail`.

## References

- Current threading: `src/rtcmix/intraverse.cpp:424-551` (MULTI_THREAD path)
- Task management: `src/rtcmix/TaskManager.h`
- Per-thread accumulation: `src/rtcmix/bus_config.cpp:637-684`
- Instrument execution: `src/rtcmix/Instrument.cpp`
- InstrumentBus implementation: `src/rtcmix/InstrumentBus.cpp`
- InstrumentBusManager: `src/rtcmix/InstrumentBusManager.cpp`
- Input path: `src/rtcmix/rtgetin.cpp`