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

    // Register/remove a consumer (tracked by Instrument pointer)
    void addConsumer(Instrument* inst);
    void removeConsumer(Instrument* inst);

    // State management
    void reset();

    // Idempotent per-cycle methods (take currentBufStart to detect dual-phase visits)
    void advanceProduction(int frames, FRAMETYPE currentBufStart);
    int prepareForIntraverseWrite(FRAMETYPE currentBufStart);
    bool hasRoomForProduction(FRAMETYPE currentBufStart) const;

    // Accessors for debugging
    int getBusID() const { return mBusID; }
    int getBufferSize() const { return mBufferSize; }
    FRAMETYPE getFramesProduced() const { return mFramesProduced; }
    int getConsumerCount() const { return (int)mConsumers.size(); }
    int getBufsamps() const { return mBufsamps; }
    int getWriteRegionStart() const { return (int)(mFramesProduced % mBufferSize); }
    int framesAvailable(Instrument* consumer) const;

private:
    int mBusID;
    int mBufsamps;           // Frames per production chunk

    // Ring buffer state (uses aux_buffer directly)
    int mBufferSize;         // Total size (INSTBUS_BUFFER_MULTIPLIER * bufsamps)
    FRAMETYPE mFramesProduced; // Total frames produced

    // Readers - tracked by Instrument pointer
    struct ConsumerState {
        FRAMETYPE framesConsumed;
        ConsumerState() : framesConsumed(0) {}
    };
    std::map<Instrument*, ConsumerState> mConsumers;

    // Run one production cycle (pops from rtQueue, computes timing, execs, re-queues)
    void runWriterCycle();

    // Ring buffer utilities
    void clearRegion(int startFrame, int numFrames);

    Lockable mPullLock;              // Serializes pullFrames/runWriterCycle per bus
    FRAMETYPE mLastPreparedAt;       // Tracks last cleared position
    FRAMETYPE mLastAdvancedBufStart; // Detects dual-phase visits within same cycle
};
```

### Phase 2: InstrumentBus Production Cycle

The InstrumentBus writer cycle pops instruments from rtQueue, computes timing,
executes, and re-queues. Processes both TO_AUX and AUX_TO_AUX queues for this bus:

```cpp
void InstrumentBus::runWriterCycle() {
    int writeStart = getWriteRegionStart();
    FRAMETYPE localBufEnd = mFramesProduced + mBufsamps;

    // 1. Clear the write region
    clearRegion(writeStart, mBufsamps);

    int busqs[2] = { mBusID, mBusID + RTcmix::busCount };  // TO_AUX, AUX_TO_AUX

    // 2. Pop and exec from both queues
    for (int q = 0; q < 2; q++) {
        RTQueue &queue = RTcmix::rtQueue[busqs[q]];
        while (queue.getSize() > 0) {
            FRAMETYPE rtQchunkStart = queue.nextChunk();
            if (rtQchunkStart >= localBufEnd) break;

            Instrument *Iptr = queue.pop(&rtQchunkStart);
            // ... compute offset, chunksamps, set_output_offset(writeStart + offset) ...
            Iptr->exec(BUS_AUX_OUT, mBusID);
        }
    }

#ifdef MULTI_THREAD
    RTcmix::mixToBus();
#endif

    // 3. Re-queue or unref after all writers complete
    // ... re-queue if endsamp > localBufEnd, else unref ...

    // 4. Advance production counter
    mFramesProduced += mBufsamps;
}
```

### Phase 3: InstrumentBus Pull Interface

```cpp
int InstrumentBus::pullFrames(Instrument* consumer, int requestedFrames) {
    AutoLock al(mPullLock);  // Serialize production per bus

    // Fast-forward if bus was idle
    if (mFramesProduced < consumer->framesConsumed)
        mFramesProduced = consumer->framesConsumed;

    // Run production cycles until we have enough frames
    while (framesAvailable(consumer) < requestedFrames) {
        runWriterCycle();
    }

    int readPos = getReadPosition(consumer, requestedFrames);
    return readPos;
}
```

### Phase 4: Integrate with Instrument Input

Modify `rtgetin()` to pull from InstrumentBus:

```cpp
int Instrument::rtgetin(float* inarr, int nsamps) {
    const int inchans = inputChannels();
    const int frames = nsamps / inchans;

    InstrumentBusManager* mgr = RTcmix::getInstBusManager();
    InstrumentBus* instBus = mgr->getInstBus(auxin[0]);

    if (instBus != NULL) {
        // Pull model: trigger production and get read position
        for (int n = 0; n < auxin_count; n++) {
            int readPos = instBus->pullFrames(this, frames);
        }
        RTcmix::readFromAuxBus(inarr, inchans, frames, auxin, auxin_count, readPos);
        return nsamps;
    }

    // Fall back to existing file/device/legacy aux input
    // ... existing rtgetin logic ...
}
```

### Phase 5: Entry Point (inTraverse)

The main loop continues to manage the heap, queues, and phased execution
(TO_AUX, AUX_TO_AUX, TO_OUT). Key changes:

- **InstrumentBus-managed aux buses** use ring buffer write positions via
  `prepareForIntraverseWrite(bufStartSamp)`
- **Production guard**: `hasRoomForProduction(bufStartSamp)` skips buses whose
  ring buffer has unconsumed data (instruments stay on rtQueue for later
  execution via `pullFrames`/`runWriterCycle`)
- **Idempotent per-cycle**: `advanceProduction(frameCount, bufStartSamp)` is
  called after each bus completes; a bus visited in both TO_AUX and AUX_TO_AUX
  only advances once, and the second phase accumulates into the same region

### Phase 6: InstrumentBus Graph Construction

Consumer registration happens in `Instrument::set_bus_config()`.
Writer execution is handled by `InstrumentBus::runWriterCycle()` which pops
instruments from rtQueue (writers are not explicitly registered).

Consumer cleanup happens in `Instrument::~Instrument()` via
`InstrumentBusManager::removeConsumer()`, which iterates all buses and erases
the instrument from each consumer map. This prevents dead consumers from
blocking production via `hasRoomForProduction()`.

## Example: WAVETABLE -> TRANS -> Output

```
Configuration:
  bus_config("WAVETABLE", "aux 0 out")         -> writes to aux0 InstrumentBus
  bus_config("TRANS", "aux 0 in", "aux 1 out") -> reads aux0, writes aux1 InstrumentBus
  bus_config("STEREO", "aux 1 in", "out 0-1")  -> reads aux1, writes output InstrumentBus

Runtime (hardware requests 512 frames):

  1. inTraverse() executes TO_AUX phase: WAVETABLE writes 512 frames to aux0
     (via TaskManager parallel exec + mixToBus)
     advanceProduction(512, bufStartSamp)

  2. inTraverse() executes AUX_TO_AUX phase for aux1:
     TRANS popped, exec'd. TRANS::run() calls rtgetin() requesting 1024 frames.

  3. rtgetin() calls aux0InstBus->pullFrames(this, 1024)
     Only 512 available. runWriterCycle() pops WAVETABLE again, produces 512 more.
     Now 1024 available. Returns readPos=0.

  4. TRANS reads 1024 from aux_buffer[0], outputs 512 to aux_buffer[1].

  5. inTraverse() executes TO_OUT phase:
     STEREO::run() calls rtgetin() requesting 512 frames from aux1.
     512 available. Returns readPos=0.

  6. STEREO writes to out_buffer. rtsendsamps() sends to hardware.
```

## What Changes vs. Current System

| Aspect | Current (Push) | InstrumentBus Model (Pull) |
|--------|----------------|----------------------------|
| Trigger | Play list order | Downstream request (via pullFrames) |
| Frame count | Fixed RTBUFSAMPS everywhere | Variable requests, fixed production |
| Buffer type | Simple arrays, cleared each cycle | aux_buffer as ring buffer (configurable persistence) |
| Threading | Per-bus: addTask->wait->mix | Per-InstrumentBus: addTask->wait->mix (same) |
| Signal flow | Implicit in play lists | Explicit in pull chain |
| intraverse role | Setup + execution | Setup + phased execution + pull fallback |

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
1. Simple chain: WAVETABLE -> aux -> MIX -> out
2. Rate change: WAVETABLE -> TRANS(2:1) -> STEREO -> out
3. Multiple sources: WAVETABLE + NOISE -> aux -> MIX -> out
4. Deep chain: A -> aux0 -> B -> aux1 -> C -> out

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
   - `pullFrames()` - triggers production cycles, returns read position
   - `runWriterCycle()` - pops from rtQueue, computes timing, executes writers
     sequentially (nested TaskManager avoided), re-queues
   - `hasRoomForProduction(FRAMETYPE currentBufStart)` - idempotent per cycle;
     returns true if already produced this cycle (dual-phase accumulation),
     otherwise checks consumers for unconsumed data
   - `prepareForIntraverseWrite(FRAMETYPE currentBufStart)` - idempotent per cycle;
     second phase returns previous write position without clearing
   - `advanceProduction(int frames, FRAMETYPE currentBufStart)` - idempotent per
     cycle; no-op on second phase visit
   - `removeConsumer(Instrument*)` - removes dead consumers under AutoLock,
     prevents `hasRoomForProduction()` from blocking on destroyed instruments
   - Full debug logging support (BBUG, WBUG, IBUG, DBUG)
   - `INSTBUS_PERSIST_DATA` / `INSTBUS_BUFFER_MULTIPLIER` for configurable persistence

2. **InstrumentBusManager class** (`src/rtcmix/InstrumentBusManager.h`, `src/rtcmix/InstrumentBusManager.cpp`)
   - Creates and manages InstrumentBus objects per aux bus
   - `getOrCreateInstBus()` - lazy creation
   - `addConsumer()` - registration
   - `removeConsumer(Instrument*)` - iterates all buses, calls `removeConsumer` on each
   - `advanceAllProduction(int frames, FRAMETYPE currentBufStart)` - advances all buses
   - Integrates with TaskManager for parallel execution

3. **Instrument integration** (`src/rtcmix/Instrument.h`, `src/rtcmix/Instrument.cpp`)
   - Consumer registration in `set_bus_config()` for auxin buses
   - Writer execution handled by `runWriterCycle()` popping from rtQueue (not registered)
   - Destructor calls `instBusMgr->removeConsumer(this)` before `_busSlot` unref,
     preventing dead consumer hang

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
   - Phased execution (TO_AUX, AUX_TO_AUX, TO_OUT) preserved
   - InstrumentBus-managed aux buses use ring buffer write positions via
     `prepareForIntraverseWrite(bufStartSamp)`
   - **Production guard**: `hasRoomForProduction(bufStartSamp)` skips buses with
     unconsumed data; instruments stay on rtQueue for later pull-based execution
   - `advanceProduction(frameCount, bufStartSamp)` called per bus after completion;
     idempotent for dual-phase buses (TO_AUX + AUX_TO_AUX same bus same cycle)
   - All timing (offsets, chunksamps, re-queue) stays in global `bufStartSamp` timeline
   - Both MULTI_THREAD and single-threaded paths updated

8. **Buffer clearing** (`src/rtcmix/buffers.cpp`)
   - `clear_aux_buffers()` skips InstrumentBus-managed buses -- they manage their own
     clearing via `clearRegion()` in `runWriterCycle()` and `prepareForIntraverseWrite()`

9. **Test scores** (`test/suite/instbus_*.sco`) - 18 test scores:
   - `instbus_basic_test.sco` - single WAVETABLE -> aux -> MIX -> out (PASS: ~10000)
   - `instbus_chain_test.sco` - WAVETABLE -> TRANS -> TRANS -> STEREO (PASS: ~5000)
   - `instbus_multi_writer_test.sco` - 2 WAVETABLEs -> same aux -> MIX -> out (PASS: ~10000)
   - `instbus_trans_test.sco` - WAVETABLE -> TRANS +1oct -> STEREO (PASS: ~5000)
   - `instbus_trans_down_test.sco` - WAVETABLE -> TRANS -1oct -> STEREO (PASS: ~5000)
   - `instbus_dual_mono_separate_test.sco` - 2 WAVETABLEs on separate aux buses (PASS: ~8000)
   - `instbus_dual_mono_to_stereo_test.sco` - 2 mono aux -> stereo output (PASS: ~8000)
   - `instbus_multilevel_merge_test.sco` - 3-level cascade, 3 sources (PASS: ~12000)
   - `instbus_multi_writer_cascade_test.sco` - 2 writers/bus at 2 stages (PASS: ~12000)
   - `instbus_deep_chain_test.sco` - 5-stage MIX chain (PASS: ~11809)
   - `instbus_asymmetric_tree_test.sco` - 3 branches depth 3/2/1 (PASS: ~12000)
   - `instbus_staggered_entry_test.sco` - 3 writers staggered t=0,1,2 (PASS: ~12000)
   - `instbus_diamond_test.sco` - diamond routing topology (FAIL: see Known Issues)
   - `instbus_parallel_merge_test.sco` - parallel paths merging (PASS: ~8000)
   - `instbus_stereo_merge_test.sco` - stereo merge routing (PASS: ~8000)
   - `instbus_quad_test.sco` - quad output routing (PASS: ~6000)
   - `instbus_complex_quad_test.sco` - complex quad routing with TRANS (PASS: ~6000)
   - `instbus_trans_reuse_test.sco` - long TRANS, staggered writers (PASS: ~5000)

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

4. **TRANS-down distortion** (FIXED): `instbus_trans_down_test.sco` had peak 5623
   vs expected 5000, with audible distortion (every other buffer skipped).
   Two problems combined:
   - `clear_aux_buffers()` ran at end of every `inTraverse` cycle and zeroed ALL
     aux buffers, destroying InstrumentBus ring buffer data not yet consumed.
   - intraverse always produced for every aux bus every cycle, overwriting data
     before TRANS-down (which only consumes every other cycle) could read it.
   **Fix (3 changes):**
   - `clear_aux_buffers()` now skips InstrumentBus-managed buses (`buffers.cpp`)
   - New `hasRoomForProduction()` method on InstrumentBus checks whether producing
     another chunk would overwrite unconsumed consumer data (`InstrumentBus.h/.cpp`)
   - intraverse skips production for InstrumentBus aux buses when
     `!hasRoomForProduction()`, leaving instruments on rtQueue for later execution
     via `pullFrames`/`runWriterCycle` (`intraverse.cpp`, both build paths)
   **Key design insight:** All timing (offsets, chunksamps, re-queue decisions)
   stays in the global `bufStartSamp` timeline. Only `output_offset` uses the
   InstrumentBus ring buffer write position.

5. **Dual-phase bus skip / asymmetric tree** (FIXED 2026-02-22): Bus 6 visited in
   both TO_AUX (WAVETABLE) and AUX_TO_AUX (MIX). After TO_AUX called
   `advanceProduction()`, `hasRoomForProduction()` returned false for AUX_TO_AUX
   because `mFramesProduced` advanced but consumer hadn't consumed yet. AUX_TO_AUX
   MIX writers were permanently skipped. Peak was 20000 vs expected 12000.
   **Fix:** Made `advanceProduction`, `hasRoomForProduction`, and
   `prepareForIntraverseWrite` idempotent per cycle via `mLastAdvancedBufStart`
   parameter. A bus visited in both phases: second visit returns true for room
   check, returns same write position without clearing, and skips the advance.

6. **Dead consumer hang / complex_quad** (FIXED 2026-02-22): TRANS instruments
   (`transdur=6.0`) outlived downstream MIX consumers (`dur=3.0`). When MIX was
   destroyed, its entry remained in `mConsumers` with stale `framesConsumed`.
   `hasRoomForProduction()` saw this dead consumer and blocked production forever.
   **Fix:** Added `InstrumentBus::removeConsumer()` (erases under `AutoLock`),
   `InstrumentBusManager::removeConsumer()` (iterates all buses), and
   `Instrument::~Instrument()` calls `mgr->removeConsumer(this)` before
   `_busSlot` unref.

### Resolved: InstrumentBus Owns Queue-Based Production Loop

**Design resolved 2026-02-14.**

The autonomous `runWriterCycle()` (using mWriters list) is replaced with a
queue-based production loop inside `pullFrames()` that pops from rtQueue,
computes timing, execs, and re-queues -- the same pattern as intraverse but
driven by demand.

**Key decisions:**
1. **No global bufEndSamp/bufStartSamp** -- output InstrumentBus's `mFramesProduced`
   is the system master clock. Each aux bus has its own timeline.
2. **Ring buffer positions derived** -- `mWritePosition` eliminated; use
   `mFramesProduced % mBufferSize`. Only absolute counters stored.
3. **Instruments re-enqueue/dequeue through rtQueue** -- each production iteration
   is a full mini-buffer-cycle. rtQueue handles all timing edge cases.
4. **InstrumentBus owns pullFrames + production loop** -- accesses rtQueue,
   TaskManager, mixToBus via RTcmix statics. No callback mechanism needed.
   Processes both TO_AUX and AUX_TO_AUX queues for its bus.
5. **Parameter timing for rate-changing chains** -- known characteristic of new
   functionality. Real-time params (MIDI, OSC) upstream of rate changers update
   at output buffer rate, not scaled. No regression (TRANS/PVOC can't read aux
   buses in push model).
6. **Heap pop V1 limitation** -- instruments starting beyond output ceiling on
   upstream buses may be delayed one buffer. Acceptable for V1.

### Known Test Failures

1. **instbus_diamond_test**: Choppy/distorted output, silence after ~1 second.

   **Topology**: WAVETABLE->aux0->{TRANS(+7semi)->aux2, TRANS(+12semi)->aux3}->MIX->aux5->STEREO->out

   **Two distinct problems identified:**

   **(a) Premature upstream consumption (silence after ~1s):**
   TRANS calls `rtgetin(in, this, RTBUFSAMPS * inchans)` multiple times per run()
   (once per 512-frame buffer exhaustion). For upward transposition (_increment>1),
   TRANS consumes input faster than it produces output. Each extra rtgetin call
   triggers `runWriterCycle` on the upstream bus, which pops WAVETABLE from rtQueue
   and advances its timeline. Result: WAVETABLE is consumed at ~2x rate, finishing
   at ~1 second instead of 2 seconds. This is fundamental to the pull model -- NOT
   specific to INSTBUS_BUFFER_MULTIPLIER.

   **(b) Sample-level discontinuities (63 jumps >2000 in steady-state):**
   Even with MULTIPLIER=2 (which prevents ring buffer overwrite), the output has
   large sample-to-sample jumps (~7469 max, expected ~263 for these frequencies).
   Peak amplitude improved from ~2100 (MULTIPLIER=1) to ~7032 (MULTIPLIER=2),
   indicating partial fix, but discontinuities remain. Root cause under investigation.

   **Open questions:**
   - Why do sample discontinuities persist with MULTIPLIER=2? The ring buffer
     positions appear correct in tracing but the output has glitches.
   - Should the pull model prevent `runWriterCycle` from running upstream instruments
     ahead of schedule? This would fix premature consumption but means rate-changing
     instruments can't get enough input in a single cycle.

2. **Channel 1 = 0 on MIX-to-output tests**: MIX with `MIX(0, 0, dur, 1, 0, 1)` outputs
   mono to ch0 only. May be expected behavior for mono aux -> stereo out routing.

### Current Debug State

All debug macros are **disabled** via `src/rtcmix/dbug.h` (IBUG, DBUG, WBUG, BBUG
all `#undef`'d). To enable, change `#undef` to `#define` in `dbug.h` or define
before including it in a specific `.cpp` file.

**Warning**: Debug output is extremely verbose. Tests will appear to hang if
debug output is active and piped through `head`/`tail`.

## References

- Current threading: `src/rtcmix/intraverse.cpp:424-551` (MULTI_THREAD path)
- Task management: `src/rtcmix/TaskManager.h`
- Per-thread accumulation: `src/rtcmix/bus_config.cpp:637-684`
- Instrument execution: `src/rtcmix/Instrument.cpp`
- InstrumentBus implementation: `src/rtcmix/InstrumentBus.cpp`
- InstrumentBusManager: `src/rtcmix/InstrumentBusManager.cpp`
- Input path: `src/rtcmix/rtgetin.cpp`
