# RTcmix Bus System: Tier-Based Pull Model Conversion

## Project Overview

This document describes the plan to convert RTcmix's audio bus routing system from a push model to a tier-based pull model. The goal is to enable instruments with different input/output frame ratios (e.g., interpolators, decimators, time-stretchers) to work correctly in instrument chains.

## The Problem with the Current Push Model

The current system pushes a fixed frame count (RTBUFSAMPS) through the entire instrument chain:

```
WAVETABLE → 1024 frames → TRANS (2:1 interpolator) → ??? → output needs 1024

Problem: TRANS receives 1024 input frames but can only produce 512 output frames.
         The output stage starves.
```

The output stage MUST receive exactly RTBUFSAMPS frames or it will starve/overflow. But pushing fixed frame counts forces all intermediate instruments to have input==output, which isn't true for rate-changing instruments.

## The Solution: Tier-Based Pull Model

### Core Concept: Tiers

A **tier** is structurally equivalent to the current bus grouping:
- A bus (e.g., aux0)
- The instruments that write to it
- A ring buffer (fixed size for any given run)
- Read cursors (one per downstream consumer)
- Write tracking (for accumulating multiple writers)

The tier uses the **same threading model** as the current per-bus execution:
- All writers run in parallel via TaskManager
- Synchronization via `waitForTasks()`
- Accumulation merged via `mixToBus()`

### Key Invariants

1. **Output guarantee**: Every instrument writes exactly `framesToRun()` frames per run
2. **Input flexibility**: Any instrument may request more or less than `framesToRun()` frames from its upstream tier

This asymmetry is the key insight:
- Production: fixed-size chunks (`framesToRun()` per instrument per run)
- Consumption: variable-size requests from downstream
- The tier's ring buffer absorbs the mismatch

### How It Works

```
True Pull:
  output requests 1024 frames from out0 tier
       ↓
  TRANS (2:1): "To output 1024, I need 2048 input frames"
       ↓
  aux0 tier: runs WAVETABLE twice (2 × 1024 = 2048 frames)
       ↓
  TRANS: reads 2048 from aux0 tier, outputs 1024
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

## Tier Architecture

### Structure

```
┌─────────────────────────────────────────────────────────┐
│                    TIER (e.g., aux0)                    │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │           Ring Buffer (fixed size)               │   │
│  │  [frame0][frame1][frame2]...[frameN]            │   │
│  │      ↑                          ↑                │   │
│  │   read cursors              write cursor         │   │
│  │   (per consumer)            (fixed chunks)       │   │
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

**Write side (fixed chunks, parallel execution):**
- Clear the next region of the ring buffer
- Add tasks for ALL input instruments to TaskManager
- `waitForTasks()` - block until all complete
- `mixToBus()` - merge per-thread accumulations into ring buffer
- Advance write cursor by `framesToRun()`

**Read side (variable requests):**
- Each consumer has its own read cursor
- Consumers request arbitrary frame counts
- Single read from ring buffer, single write to consumer's input buffer
- Read cursors advance independently

**Safe to overwrite:**
```
oldest_read = min(all consumer read cursors)
can_overwrite_up_to = oldest_read
```

### Timeline Alignment

For instruments reading from aux buses (not file input):
- All consumers are time-synchronized
- Frame N in the tier buffer represents the same moment for all readers
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
#undef TBUG      // NEW: Tier debugging
```

All new code must include equivalent debug logging:

```cpp
#ifdef TBUG
    printf("Tier::pullFrames(consumer=%d, requested=%d) entering\n",
           consumerID, requestedFrames);
#endif

#ifdef WBUG
    printf("ENTERING Tier::runWriterCycle()\n");
#endif

#ifdef IBUG
    printf("Tier: adding inst %p to taskmgr [%s]\n", inst, inst->name());
#endif
```

## Implementation Plan

### Phase 1: Tier Class

Create new `Tier` class that wraps a bus with ring buffer management.
Must support both MULTI_THREAD and single-threaded builds:

```cpp
class Tier {
public:
    Tier(int busID, int bufferSize, int numChannels);
    ~Tier();

    // Called by downstream instruments to request input
    int pullFrames(int consumerID, int requestedFrames, BufPtr dest);

    // Register a consumer, returns consumerID
    int addConsumer();

    // Register an input instrument
    void addWriter(Instrument* inst);

    // Accessors for debugging
    int getBusID() const { return busID; }
    int getWritePosition() const { return writePosition; }
    int getFramesProduced() const { return framesProduced; }

private:
    int busID;
    int numChannels;

    // Ring buffer
    BufPtr ringBuffer;
    int bufferSize;          // Total size in frames
    int writePosition;       // Current write position
    int framesProduced;      // Total frames produced (for availability calc)

    // Writers (instruments that produce to this tier)
    std::vector<Instrument*> writers;

    // Readers (downstream consumers)
    std::vector<int> readCursors;    // Per-consumer read position
    std::vector<int> framesConsumed; // Per-consumer total consumed

    // Run one production cycle (handles both threaded and non-threaded)
    void runWriterCycle();

    // How many frames available for a given consumer
    int framesAvailable(int consumerID);

    // Ring buffer utilities
    void clearRegion(int start, int length);
    void copyToConsumer(BufPtr dest, int readPos, int frames);

#ifdef MULTI_THREAD
    // Reference to global TaskManager (or could be passed in)
    TaskManager* taskManager;
#endif
};
```

### Phase 2: Tier Production Cycle

The tier's writer cycle mirrors the current per-bus pattern, with both MULTI_THREAD and single-threaded paths:

```cpp
void Tier::runWriterCycle() {
    // Matches current intraverse.cpp pattern for a single bus

#ifdef WBUG
    printf("ENTERING Tier::runWriterCycle() for bus %d\n", busID);
#endif

#ifdef MULTI_THREAD
    vector<Instrument*> instruments;
#endif

    // 1. Clear the ring buffer region we're about to write
    clearRegion(writePosition, framesToRun());

#ifdef TBUG
    printf("Tier %d: cleared region [%d, %d)\n",
           busID, writePosition, writePosition + framesToRun());
#endif

    // 2. Execute all writers
    for (Instrument* inst : writers) {
        // Set up instrument for this chunk
        inst->setchunk(framesToRun());
        inst->set_output_offset(0);  // Writing to start of cleared region

#ifdef IBUG
        printf("Tier %d: processing inst %p [%s]\n", busID, inst, inst->name());
#endif

#ifdef MULTI_THREAD
        instruments.push_back(inst);
        taskManager->addTask<Instrument, int, BusType, int, &Instrument::exec>
            (inst, BUS_AUX_OUT, busID);
#else
        // Single-threaded: execute directly
        inst->exec(BUS_AUX_OUT, busID);
#endif
    }

#ifdef MULTI_THREAD
    // 3. Wait for all writers to complete
    if (!instruments.empty()) {
#ifdef DBUG
        printf("Tier %d: waiting for %d instrument tasks\n",
               busID, (int)instruments.size());
#endif
        taskManager->waitForTasks(instruments);

        // 4. Merge per-thread accumulations into ring buffer
        RTcmix::mixToBus();
    }
#endif

    // 5. Advance write position
    writePosition = (writePosition + framesToRun()) % bufferSize;
    framesProduced += framesToRun();

#ifdef TBUG
    printf("Tier %d: writePosition now %d, framesProduced=%d\n",
           busID, writePosition, framesProduced);
#endif

    // 6. Re-queue writers if they have more to produce
#ifdef MULTI_THREAD
    for (Instrument* inst : instruments) {
#else
    for (Instrument* inst : writers) {
#endif
        // ... re-queue or unref logic ...
    }

#ifdef WBUG
    printf("EXITING Tier::runWriterCycle() for bus %d\n", busID);
#endif
}
```

### Phase 3: Tier Pull Interface

```cpp
int Tier::pullFrames(int consumerID, int requestedFrames, BufPtr dest) {
#ifdef WBUG
    printf("ENTERING Tier::pullFrames(bus=%d, consumer=%d, frames=%d)\n",
           busID, consumerID, requestedFrames);
#endif

#ifdef TBUG
    printf("Tier %d: consumer %d requesting %d frames, available=%d\n",
           busID, consumerID, requestedFrames, framesAvailable(consumerID));
#endif

    // Loop until we have enough frames
    int cycleCount = 0;
    while (framesAvailable(consumerID) < requestedFrames) {
#ifdef TBUG
        printf("Tier %d: need more frames, running cycle %d\n",
               busID, ++cycleCount);
#endif
        runWriterCycle();  // Runs writers, waits, mixes, advances
    }

#ifdef TBUG
    printf("Tier %d: copying %d frames from readPos=%d to consumer %d\n",
           busID, requestedFrames, readCursors[consumerID], consumerID);
#endif

    // Copy requested frames to consumer's buffer
    copyToConsumer(dest, readCursors[consumerID], requestedFrames);

    // Advance this consumer's read cursor
    int oldPos = readCursors[consumerID];
    readCursors[consumerID] =
        (readCursors[consumerID] + requestedFrames) % bufferSize;

#ifdef TBUG
    printf("Tier %d: consumer %d readCursor %d -> %d\n",
           busID, consumerID, oldPos, readCursors[consumerID]);
#endif

#ifdef WBUG
    printf("EXITING Tier::pullFrames(bus=%d) returning %d\n",
           busID, requestedFrames);
#endif

    return requestedFrames;
}
```

### Phase 4: Integrate with Instrument Input

Modify `rtgetin()` to pull from tier:

```cpp
int Instrument::rtgetin(float* inarr, int requestedSamps) {
    int requestedFrames = requestedSamps / inputChannels();

    if (inputTier != nullptr) {
        // Pull from upstream tier
        return inputTier->pullFrames(myConsumerID, requestedFrames, inarr)
               * inputChannels();
    }

    // Fall back to existing file/device input
    return existingRtgetinLogic(inarr, requestedSamps);
}
```

### Phase 5: Entry Point (inTraverse)

The main loop becomes a simple pull from the output tier:

```cpp
bool RTcmix::inTraverse(AudioDevice* device, void* arg) {
    // ... existing setup, heap popping, etc. ...

    // Pull RTBUFSAMPS frames from output tier
    // This triggers cascading pulls through all upstream tiers
    outputTier->pullFrames(HARDWARE_CONSUMER_ID, RTBUFSAMPS, out_buffer);

    // Send to hardware (unchanged)
    rtsendsamps(device);

    // ... existing cleanup ...
    return playEm;
}
```

### Phase 6: Tier Graph Construction

In `bus_config()`, build the tier graph:

```cpp
double RTcmix::bus_config(double p[], int n_args) {
    // ... existing parsing ...

    // Create or get tier for each output bus
    for (int i = 0; i < bus_slot->auxout_count; i++) {
        Tier* tier = getOrCreateTier(bus_slot->auxout[i]);
        tier->addWriter(currentInstrument);
    }

    // Register as consumer of input tiers
    for (int i = 0; i < bus_slot->auxin_count; i++) {
        Tier* tier = getOrCreateTier(bus_slot->auxin[i]);
        int consumerID = tier->addConsumer();
        currentInstrument->setInputTier(tier, consumerID);
    }
}
```

## Example: WAVETABLE → TRANS → Output

```
Configuration:
  bus_config("WAVETABLE", "in0", "aux0out")  → writes to aux0 tier
  bus_config("TRANS", "aux0in", "out0")      → reads aux0, writes out0 tier

Runtime (hardware requests 1024 frames):

  1. inTraverse() calls outputTier->pullFrames(1024)

  2. outputTier needs TRANS to produce 1024 frames
     - Runs TRANS via runWriterCycle()
     - TRANS::run() calls rtgetin() requesting 2048 frames

  3. rtgetin() calls aux0Tier->pullFrames(2048)

  4. aux0Tier needs 2048 frames, has 0 available
     Loop iteration 1:
       - clearRegion(0, 1024)
       - addTask(WAVETABLE)
       - waitForTasks()
       - mixToBus()
       - writePosition = 1024

     Loop iteration 2:
       - clearRegion(1024, 1024)
       - addTask(WAVETABLE)
       - waitForTasks()
       - mixToBus()
       - writePosition = 2048

     Now 2048 frames available

  5. aux0Tier copies 2048 frames to TRANS input buffer

  6. TRANS processes 2048 → 1024 output frames

  7. outputTier has 1024 frames, copies to out_buffer

  8. rtsendsamps() sends to hardware
```

## What Changes vs. Current System

| Aspect | Current (Push) | Tier Model (Pull) |
|--------|----------------|-------------------|
| Trigger | Play list order | Downstream request |
| Frame count | Fixed RTBUFSAMPS everywhere | Variable requests, fixed production |
| Buffer type | Simple arrays, cleared each cycle | Ring buffers, persistent |
| Threading | Per-bus: addTask→wait→mix | Per-tier: addTask→wait→mix (same) |
| Signal flow | Implicit in play lists | Explicit in pull chain |

## What Stays Unchanged

- `Instrument::run()` - generates samples into private `outbuf`
- `Instrument::exec()` - run + addout coordination
- `Instrument::rtaddout()` - writes to private `outbuf`
- `Instrument::addout()` - queues mix operation
- `addToBus()` - per-thread accumulation (MULTI_THREAD) or direct (single-threaded)
- `mixToBus()` - merge thread-local vectors (MULTI_THREAD only)
- `TaskManager` - parallel execution infrastructure (MULTI_THREAD only)
- `heap` scheduling system
- Instrument lifecycle (ref counting, re-queuing)
- **Build configurations** - both MULTI_THREAD and single-threaded must work
- **Debug macro pattern** - DBUG, IBUG, WBUG, BBUG, ALLBUG

## Testing Strategy

### Build Configuration Tests
1. **MULTI_THREAD build** - verify compilation and execution
2. **Single-threaded build** - verify compilation and execution
3. **Debug builds** - verify all debug macros compile (DBUG, IBUG, WBUG, BBUG, TBUG, ALLBUG)
4. **EMBEDDED build** - verify compatibility

### Unit Tests
1. Single writer, single reader tier
2. Multiple writers to same tier (verify parallel exec + mixing)
3. Multiple readers from same tier (verify independent cursors)
4. Request larger than single production cycle (verify looping)
5. Request smaller than production (verify buffering)

### Integration Tests
1. Simple chain: WAVETABLE → MIX → out
2. Rate change: WAVETABLE → TRANS(2:1) → out
3. Multiple sources: WAVETABLE + NOISE → aux → MIX → out
4. Deep chain: A → aux0 → B → aux1 → C → out

### Regression Tests
- All existing test scores in `test/suite/`
- Verify TRANS, PVOC work correctly in chains
- **Run all tests in both MULTI_THREAD and single-threaded builds**

## Buffer Sizing

Tier ring buffers must accommodate:
- Maximum single request from any consumer
- Enough headroom for multiple production cycles

Conservative default: `4 * RTBUFSAMPS` per tier

The output tier is special: exactly `RTBUFSAMPS` (must match hardware).

## Implementation Status

### Completed (Phase 1)

The following infrastructure has been implemented:

1. **Tier class** (`src/rtcmix/Tier.h`, `src/rtcmix/Tier.cpp`)
   - Ring buffer management with configurable size
   - Per-consumer read cursors for independent consumption
   - Writer registration and removal
   - `pullFrames()` - triggers production cycles to satisfy requests
   - `runWriterCycle()` - executes writers in parallel (MULTI_THREAD) or sequentially
   - Full debug logging support (TBUG, WBUG, IBUG, DBUG)

2. **TierManager class** (`src/rtcmix/TierManager.h`, `src/rtcmix/TierManager.cpp`)
   - Creates and manages Tier objects per aux bus
   - Registers writers and consumers
   - Integrates with TaskManager for parallel execution

3. **Instrument integration** (`src/rtcmix/Instrument.h`, `src/rtcmix/Instrument.cpp`)
   - `inputTier` and `inputTierConsumerID` fields
   - `setInputTier()` method for tier configuration
   - Registration in `set_bus_config()` for aux buses

4. **Input path modification** (`src/rtcmix/rtgetin.cpp`)
   - Checks for `hasInputTier()` and pulls from tier if available
   - Falls back to legacy aux_buffer path otherwise

5. **RTcmix integration** (`src/rtcmix/RTcmix.h`, `src/rtcmix/RTcmix.cpp`)
   - Static `tierManager` member
   - Initialization in `init_globals()`
   - Cleanup in `free_globals()`
   - `getTierManager()` accessor

6. **Test scores** (`test/suite/tier_*.sco`)
   - `tier_basic_test.sco` - simple WAVETABLE → aux → MIX chain
   - `tier_chain_test.sco` - multi-tier cascade
   - `tier_multi_writer_test.sco` - multiple writers to same tier

### Remaining Work (Phase 2)

The tier infrastructure is in place, but the following integration is needed for full pull-model operation:

1. **intraverse.cpp modification**
   - The main audio loop still uses the push model
   - Need to add tier-based triggering for aux-bus instruments
   - When an instrument with `auxin` needs input, its tier should be signaled

2. **Output tier integration**
   - Create a special output tier that pulls from the entire graph
   - Trigger from `inTraverse()` main loop

3. **Writer lifecycle management**
   - Remove writers when instruments finish (`unref()`)
   - Handle re-queuing of instruments that span multiple buffers

### Current Behavior

With the current implementation:
- Instruments register with tiers at `set_bus_config()` time
- Tiers are created for each aux bus
- The `rtgetin()` path can pull from tiers
- However, since `runWriterCycle()` isn't triggered from the main loop, the system still relies on the push model through `aux_buffer`

The infrastructure supports the pull model, but full activation requires intraverse.cpp changes.

## References

- Current threading: `src/rtcmix/intraverse.cpp:424-551` (MULTI_THREAD path)
- Task management: `src/rtcmix/TaskManager.h`
- Per-thread accumulation: `src/rtcmix/bus_config.cpp:637-684`
- Instrument execution: `src/rtcmix/Instrument.cpp:328-358`
- Tier implementation: `src/rtcmix/Tier.cpp`
- TierManager: `src/rtcmix/TierManager.cpp`