# RTcmix Bus System: InstrumentBus-Based Pull Model Conversion

##	C++ Coding rules
1. All simple class member accessor functions should be inlined.
2. Use stdlib utilities when modifying the contents of stdlib containers - do not use iterator loops if possible.
3. Always favor *behavior* over conditional statements - the fewer if/else statements, the better.
4. Keep logic as simple as possible.
5. If outside state is needed for a single class method, pass it in via arguments rather than making it a member of the class.
6. Use asserts rather than conditionals when checking for state: If the state has only one valid value in a correctly-functioning system, assert on that value.
7. Use wrapped versions of primatives (locks, dylibs, etc.) which are available in the code base whenever possible.
8. Limit yourself to the following source files and their direct #includes.  If you need to read any other file, ask permission.

```
InstrumentBusManager.h
InstrumentBus.h
intraverse.cpp
InstrumentBus.cpp
buffers.cpp
rtgetin.cpp
dbug.h
bus_config.cpp
TaskManager.h
TaskManager.cpp
RTcmix.h
RTThread.h
RTThread.cpp
InstrumentBusManager.cpp
Instrument.cpp
Instrument.h
```


## Project Overview

This document describes the conversion of RTcmix's audio bus routing system from a push model to an InstrumentBus-based pull model. The goal is to enable instruments with different input/output frame ratios (e.g., interpolators, decimators, time-stretchers) to work correctly in instrument chains.

For a detailed architecture walkthrough, see `project_arch.md`.

## The Problem with the Push Model

The original system pushes a fixed frame count (RTBUFSAMPS) through the entire instrument chain:

```
WAVETABLE -> 1024 frames -> TRANS (2:1 interpolator) -> ??? -> output needs 1024

Problem: TRANS receives 1024 input frames but can only produce 512 output frames.
         The output stage starves.
```

The output stage MUST receive exactly RTBUFSAMPS frames or it will starve/overflow. But pushing fixed frame counts forces all intermediate instruments to have input==output, which isn't true for rate-changing instruments.

## The Solution: Hybrid Push/Pull Model

### Core Concept

The system uses a **hybrid push/pull** model:

- **Push path (intraverse):** The main audio callback processes buses in phase
  order (TO_AUX, AUX_TO_AUX, TO_OUT) using `TaskManager` for parallel execution.
  This handles the common 1:1 frame ratio case with zero overhead.

- **Pull path (InstrumentBus):** When a downstream instrument calls `rtgetin()`
  and the input comes from an InstrumentBus-managed aux bus, `pullFrames()` is
  called. If insufficient frames are available, `runWriterCycle()` pops instruments
  from `rtQueue` and executes them on demand.

### Key Invariants

1. **Output guarantee**: Every instrument writes exactly `framesToRun()` frames per run
2. **Input flexibility**: Any instrument may request more or less than `framesToRun()` frames from its upstream tier

The InstrumentBus ring buffer absorbs the mismatch between fixed-size production
and variable-size consumption.

## Pre-Existing Architecture Reference

### Key Files

| File | Purpose |
|------|---------|
| `src/rtcmix/bus_config.cpp` | Bus configuration, play order, addToBus(), mixToBus() |
| `src/rtcmix/intraverse.cpp` | Main audio loop, scheduling, task management |
| `src/rtcmix/Instrument.cpp` | Base instrument class, exec/run/addout |
| `src/rtcmix/buffers.cpp` | Buffer allocation and clearing |
| `src/rtcmix/rtgetin.cpp` | Input reading (pull-based entry point) |
| `src/rtcmix/TaskManager.h` | Thread pool task management |
| `src/rtcmix/InstrumentBus.h/.cpp` | Ring buffer wrapper, pull/production logic |
| `src/rtcmix/InstrumentBusManager.h/.cpp` | Bus creation, consumer routing |

### Threading Model (MULTI_THREAD)

The push path processes each bus using the shared static helpers:

```cpp
// For each bus in current phase (TO_AUX, AUX_TO_AUX, TO_OUT):

// 1. Pop and prepare all instruments (shared helper — no execution)
instruments = InstrumentBus::popAndPrepareWriters(
    busq, bufStartSamp, bufEndSamp, instBusWriteStart, frameCount, panic);

// 2. Execute in parallel via TaskManager
for (auto *Iptr : instruments)
    taskManager->addTask<...>(Iptr, bus_type, bus);
taskManager->waitForTasks(instruments);

// 3. Merge per-thread accumulation vectors into bus buffer
RTcmix::mixToBus();

// 4. Re-queue or unref instruments (shared helper)
InstrumentBus::requeueOrUnref(instruments, bufEndSamp, busq, qStatus, panic);
rtQueue[busq].sort();
```

### Thread-Safe Accumulation

In `MULTI_THREAD` mode, `addToBus()` queues mix operations per-thread:

```cpp
void RTcmix::addToBus(BusType type, int bus, BufPtr src, int offset, int endfr, int chans)
{
    mixVectors[RTThread::GetIndexForThread()].push_back(
        MixData(src, dest, endfr - offset, chans)
    );
}
```

Then `mixToBus()` merges all thread-local vectors:

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

| Instrument | Input Pattern | Output Pattern |
|------------|---------------|----------------|
| TRANS | Variable (interpolation ratio) | Fixed `framesToRun()` |
| PVOC | `_decimation` frames per hop | Fixed `framesToRun()` |
| LPCIN | Variable `_counter` (pitch-dependent) | Fixed `framesToRun()` |
| WAVETABLE | None (generator) | Fixed `framesToRun()` |
| MIX | Fixed `framesToRun()` | Fixed `framesToRun()` |

## As-Built Design

### InstrumentBus Class

One per aux bus.  Wraps `RTcmix::aux_buffer[busID]` as a ring buffer.
No separate buffer allocation -- uses `aux_buffer` directly.

```cpp
class InstrumentBus {
public:
    InstrumentBus(int busID, int bufsamps);
    ~InstrumentBus();

    // Pull interface: trigger production and get read position
    int pullFrames(Instrument* consumer, int requestedFrames);

    // Consumer lifecycle
    void addConsumer(Instrument* inst);
    void removeConsumer(Instrument* inst);

    // Idempotent per-cycle methods (safe for dual-phase bus visits)
    void advanceProduction(int frames, FRAMETYPE currentBufStart);
    int  prepareForIntraverseWrite(FRAMETYPE currentBufStart);
    bool hasRoomForProduction(FRAMETYPE currentBufStart) const;

    // Accessors (all inlined)
    int getBusID() const;
    int getBufferSize() const;
    FRAMETYPE getFramesProduced() const;
    int getConsumerCount() const;
    int getBufsamps() const;
    int getWriteRegionStart() const;  // mFramesProduced % mBufferSize

    int framesAvailable(Instrument* consumer) const;
    int getReadPosition(Instrument* consumer, int frames);

    // Shared scheduling helpers (static — used by both push and pull paths)
    static std::vector<Instrument*> popAndPrepareWriters(
        int busq, FRAMETYPE timelineOrigin, FRAMETYPE bufEnd,
        int writeStart, int maxFrames, bool panic);
    static void requeueOrUnref(std::vector<Instrument*> &instsToRun,
        FRAMETYPE bufEnd, int busq, IBusClass qStatus, bool panic);

    void reset();

private:
    int mBusID, mBufsamps, mBufferSize;
    FRAMETYPE mFramesProduced;           // monotonic production counter

    struct ConsumerState { FRAMETYPE framesConsumed; };
    std::map<Instrument*, ConsumerState> mConsumers;

    void runWriterCycle();               // pull-path orchestrator
    void clearRegion(int startFrame, int numFrames);

    Lockable mPullLock;                  // serializes pullFrames per bus
    FRAMETYPE mLastAdvancedBufStart;     // dedup dual-phase production
};
```

**Key design:** Writers are NOT registered with InstrumentBus.  Both intraverse
(push) and `runWriterCycle()` (pull) pop writers from `rtQueue` at execution time.

**Shared scheduling helpers:** `popAndPrepareWriters` and `requeueOrUnref` are
static methods that contain the scheduling logic formerly duplicated between
intraverse and InstrumentBus.  They are parameterized by timeline origin,
buffer end bound, write position, and queue index — values that differ between
the push and pull paths.  Neither method executes instruments; the caller
chooses parallel (TaskManager) or sequential execution.

### InstrumentBusManager Class

Singleton owned by RTcmix.  Manages all InstrumentBus objects.

```cpp
class InstrumentBusManager {
public:
    InstrumentBusManager(int busCount, int bufsamps);
    ~InstrumentBusManager();

    InstrumentBus* getOrCreateInstBus(int busID);  // lazy creation
    InstrumentBus* getInstBus(int busID) const;     // NULL if not created

    void addConsumer(int busID, Instrument* inst);
    void removeConsumer(Instrument* inst);           // iterates all buses
    void advanceAllProduction(int frames, FRAMETYPE currentBufStart);
    void reset();
    int getActiveInstBusCount() const { return (int)mInstBuses.size(); }

private:
    std::map<int, InstrumentBus*> mInstBuses;  // keyed by bus ID
    int mBufsamps;
};
```

### Consumer Lifecycle

**Registration** (parse time, `Instrument::set_bus_config()`):
```cpp
for (int i = 0; i < _busSlot->auxin_count; i++)
    instBusMgr->addConsumer(_busSlot->auxin[i], this);
```

**Removal** (destruction, `Instrument::~Instrument()`):
```cpp
RTcmix::getInstBusManager()->removeConsumer(this);
```

Removal prevents dead consumers from blocking `hasRoomForProduction()`.

### Pull Path (rtgetin -> pullFrames -> runWriterCycle)

When an instrument reads from an aux bus, `rtgetin()` pulls via InstrumentBus:

```cpp
// In Instrument::rtgetin() -- aux bus input path
int readPos = 0;
for (int n = 0; n < auxin_count; n++) {
    int chan = auxin[n];
    InstrumentBus *instBus = RTcmix::getInstBus(chan);
    assert(instBus != NULL);
    readPos = instBus->pullFrames(this, frames);
}
RTcmix::readFromAuxBus(inarr, inchans, frames, auxin, auxin_count, readPos);
```

Every aux bus with consumers has an InstrumentBus (created at `addConsumer` time),
so the assert is always satisfied.  There is no legacy fallback path.

`pullFrames()` triggers production if needed:

```cpp
int InstrumentBus::pullFrames(Instrument* consumer, int requestedFrames) {
    AutoLock al(mPullLock);
    // Fast-forward idle bus if needed
    if (mFramesProduced < consumer->framesConsumed)
        mFramesProduced = consumer->framesConsumed;
    // Produce until enough frames available
    while (framesAvailable(consumer) < requestedFrames)
        runWriterCycle();
    return getReadPosition(consumer, requestedFrames);
}
```

`runWriterCycle()` uses the shared static helpers per queue:

```cpp
void InstrumentBus::runWriterCycle() {
    int writeStart = getWriteRegionStart();
    FRAMETYPE localBufEnd = mFramesProduced + mBufsamps;
    clearRegion(writeStart, mBufsamps);

    int busqs[2] = { mBusID, mBusID + busCount };
    for (int q = 0; q < 2; q++) {
        auto insts = popAndPrepareWriters(busqs[q], mFramesProduced,
                                           localBufEnd, writeStart,
                                           mBufsamps, panic);
        // Execute sequentially (pull path — no TaskManager)
        for (auto *Iptr : insts) Iptr->exec(BUS_AUX_OUT, mBusID);
        RTcmix::mixToBus();
        requeueOrUnref(insts, localBufEnd, busqs[q], UNKNOWN, panic);
        RTcmix::rtQueue[busqs[q]].sort();
    }
    mFramesProduced += mBufsamps;
}
```

Each queue is processed independently: pop+prepare, execute sequentially,
mix, requeue with the explicit queue index.  `UNKNOWN` as `qStatus` means
"always unref if finished" (no phase guard needed in the pull path).

### Push Path Integration (intraverse)

The push path uses the shared static helpers for scheduling, plus per-bus
InstrumentBus methods for ring buffer management:

```cpp
// InstrumentBus ring buffer setup (aux buses only)
int instBusWriteStart = 0;
InstrumentBus *instBus = (bus_type == BUS_AUX_OUT) ? getInstBus(bus) : NULL;
if (instBus) {
    if (!instBus->hasRoomForProduction(bufStartSamp)) {
        continue;  // instruments stay on rtQueue for later pull
    }
    instBusWriteStart = instBus->prepareForIntraverseWrite(bufStartSamp);
}

// Pop and prepare (shared static helper — works for all bus types)
instruments = InstrumentBus::popAndPrepareWriters(
    busq, bufStartSamp, bufEndSamp, instBusWriteStart, frameCount, panic);

// Execute via TaskManager (parallel push path)
for (auto *Iptr : instruments)
    taskManager->addTask<Instrument, int, BusType, int, &Instrument::exec>(
        Iptr, bus_type, bus);

// Wait + mix + advance
if (!instruments.empty()) {
    taskManager->waitForTasks(instruments);
    RTcmix::mixToBus();
    if (instBus) instBus->advanceProduction(frameCount, bufStartSamp);
}

// Requeue or unref (shared static helper)
InstrumentBus::requeueOrUnref(instruments, bufEndSamp, busq, qStatus, panic);
rtQueue[busq].sort();
```

The static helpers handle all bus types (TO_AUX, AUX_TO_AUX, TO_OUT) uniformly.
`instBusWriteStart` is 0 for non-InstrumentBus buses, which is the correct
base offset for normal `aux_buffer`/`out_buffer` writes.

### Dual-Phase Idempotency

A bus can appear in both TO_AUX and AUX_TO_AUX playlists.  Three methods use
`mLastAdvancedBufStart` to make this safe:

| Method | First phase | Second phase (same cycle) |
|--------|-------------|---------------------------|
| `hasRoomForProduction(bss)` | Normal consumer check | Returns `true` |
| `prepareForIntraverseWrite(bss)` | Clears, returns writePos | Returns same writePos, no clear |
| `advanceProduction(n, bss)` | Advances `mFramesProduced` | No-op |

### Buffer Clearing

`clear_aux_buffers()` (called at end of every `inTraverse`) skips
InstrumentBus-managed buses.  They manage their own clearing via
`clearRegion()` in `runWriterCycle()` and `prepareForIntraverseWrite()`.

## Build Configuration Requirements

### Dual Compilation: MULTI_THREAD and Single-Threaded

Both build modes are supported:

**MULTI_THREAD mode:**
- Uses TaskManager for parallel instrument execution in the push path
- Per-thread accumulation via `mixVectors[]`
- `waitForTasks()` synchronization, `mixToBus()` to merge
- Pull path (`runWriterCycle`) executes writers sequentially to avoid nested parallelism

**Single-threaded mode:**
- Direct sequential instrument execution
- Direct accumulation into bus buffer (no mixVectors)
- No TaskManager dependency

### Debug Logging Macros

All controlled via `src/rtcmix/dbug.h`:

```cpp
#undef ALLBUG    // All debug output
#undef BBUG      // Bus debugging (verbose!)
#undef DBUG      // General debug
#undef WBUG      // "Where we are" prints
#undef IBUG      // Instrument and InstrumentBus debugging
```

All InstrumentBus code includes equivalent debug logging.

**Warning**: Debug output is extremely verbose. Tests will appear to hang if
debug output is active and piped through `head`/`tail`.

## Testing

For build rules, test commands, and quick verification, see `project_testing.md`.

### Test Scores (`test/suite/instbus_*.sco`) -- 18 tests

Run all: `cd test/suite && make instbus_tests`

| Test | Topology | Status |
|------|----------|--------|
| `instbus_basic_test` | WAVETABLE -> aux -> MIX -> out | PASS (~10000) |
| `instbus_chain_test` | WAVETABLE -> TRANS -> TRANS -> STEREO | PASS (~5000) |
| `instbus_multi_writer_test` | 2x WAVETABLE -> aux -> MIX -> out | PASS (~10000) |
| `instbus_trans_test` | WAVETABLE -> TRANS +1oct -> STEREO | PASS (~5000) |
| `instbus_trans_down_test` | WAVETABLE -> TRANS -1oct -> STEREO | PASS (~5000) |
| `instbus_trans_reuse_test` | Long TRANS, staggered writers | PASS (~5000) |
| `instbus_dual_mono_separate_test` | 2 WAVETABLEs on separate aux buses | PASS (~8000) |
| `instbus_dual_mono_to_stereo_test` | 2 mono aux -> stereo output | PASS (~8000) |
| `instbus_multilevel_merge_test` | 3-level cascade, 3 sources | PASS (~12000) |
| `instbus_multi_writer_cascade_test` | 2 writers/bus at 2 stages | PASS (~12000) |
| `instbus_deep_chain_test` | 5-stage MIX chain | PASS (~11809) |
| `instbus_asymmetric_tree_test` | 3 branches depth 3/2/1 | PASS (~12000) |
| `instbus_staggered_entry_test` | 3 writers staggered t=0,1,2 | PASS (~12000) |
| `instbus_parallel_merge_test` | Parallel paths merging | PASS (~8000) |
| `instbus_stereo_merge_test` | Stereo merge routing | PASS (~8000) |
| `instbus_quad_test` | Quad output routing | PASS (~6000) |
| `instbus_complex_quad_test` | Complex quad with TRANS | PASS (~6000) |
| `instbus_diamond_test` | Diamond: source splits to 2 TRANS, merges | **FAIL** (see below) |

**17 of 18 tests pass.**

### Known Test Failure

**instbus_diamond_test**: Choppy/distorted output, silence after ~1 second.

Topology: `WAVETABLE->aux0->{TRANS(+7semi)->aux2, TRANS(+12semi)->aux3}->MIX->aux5->STEREO->out`

Two distinct problems:

**(a) Premature upstream consumption (silence after ~1s):**
TRANS calls `rtgetin()` multiple times per `run()` (once per 512-frame buffer
exhaustion). For upward transposition (`_increment>1`), TRANS consumes input
faster than it produces output. Each extra `rtgetin` call triggers
`runWriterCycle` on the upstream bus, which pops WAVETABLE from rtQueue and
advances its timeline. Result: WAVETABLE is consumed at ~2x rate, finishing at
~1 second instead of 2 seconds. This is fundamental to the pull model -- NOT
specific to `INSTBUS_BUFFER_MULTIPLIER`.

**(b) Sample-level discontinuities:**
Even with `MULTIPLIER=2` (which prevents ring buffer overwrite), the output has
large sample-to-sample jumps. Root cause under investigation.

### Other Notes

**Channel 1 = 0 on MIX-to-output tests**: MIX with `MIX(0, 0, dur, 1, 0, 1)`
outputs mono to ch0 only. May be expected behavior for mono aux -> stereo out
routing.

## Resolved Issues

1. **Double execution** (FIXED): intraverse phased execution preserved.  TO_AUX
   and AUX_TO_AUX instruments execute via the push path with TaskManager.  The
   pull path (`runWriterCycle`) only runs when additional production is needed
   beyond what the push path provided.

2. **Crash from unconfigured instruments** (FIXED): Writer execution is handled
   by popping from rtQueue (not by explicit registration), so only instruments
   that have been configured and reached their start time are executed.

3. **STEREO pan issue** (FIXED): `instbus_dual_mono_to_stereo_test.sco` was using
   `STEREO(0, 0, dur, 1, 0.5)` with 2 input channels but only 1 pan pfield.
   Fixed to `STEREO(0, 0, dur, 1, 1.0, 0.0)`.

4. **TRANS-down distortion** (FIXED): Two problems combined:
   - `clear_aux_buffers()` zeroed ALL aux buffers, destroying ring buffer data.
   - intraverse always produced for every aux bus, overwriting data before
     TRANS-down (which only consumes every other cycle) could read it.
   Fix: `clear_aux_buffers()` skips managed buses; `hasRoomForProduction()`
   defers production when ring buffer has unconsumed data.

5. **Dual-phase bus skip / asymmetric tree** (FIXED 2026-02-22): Bus visited in
   both TO_AUX and AUX_TO_AUX: after TO_AUX called `advanceProduction()`,
   `hasRoomForProduction()` returned false for AUX_TO_AUX because
   `mFramesProduced` advanced but consumer hadn't consumed yet.
   Fix: Made `advanceProduction`, `hasRoomForProduction`, and
   `prepareForIntraverseWrite` idempotent per cycle via `mLastAdvancedBufStart`
   parameter (rule 5: pass outside state as argument).

6. **Dead consumer hang / complex_quad** (FIXED 2026-02-22): TRANS instruments
   outlived downstream MIX consumers. When MIX was destroyed, its stale entry
   in `mConsumers` blocked `hasRoomForProduction()` forever.
   Fix: Added `removeConsumer()` to InstrumentBus and InstrumentBusManager;
   `Instrument::~Instrument()` calls `mgr->removeConsumer(this)`.

7. **Stress test "scheduler behind" warnings** (FIXED 2026-02-23): The
   interactive stress test (`test/suite/stresstest`) printed thousands of
   "WARNING: the scheduler is behind the queue!" messages. Root cause: the
   `stresstest` binary was compiled against stale headers. The Makefile in
   `test/suite/` does not track header dependencies, so changes to `RTcmix.h`
   did not trigger recompilation. Fix: `rm -f stresstest.o stresstest &&
   make stresstest`. Added rebuild step to `project_testing.md`.

## Code Cleanup Applied (2026-02-23)

The following cleanup was applied to all InstrumentBus-related source files
per the C++ coding rules above:

1. Decomposed `runWriterCycle()` into orchestrator + `popAndExecWriters()` +
   `requeueOrUnref()`. The re-queue helper derives `busq` from the instrument's
   `BusSlot::Class()` instead of storing it separately.
2. Inlined `getActiveInstBusCount()` in InstrumentBusManager.h.
3. Converted all container iteration to `std::for_each` with C++11 lambdas
   (InstrumentBusManager methods, InstrumentBus::reset).
4. Changed `InstrumentBusManager::mInstBuses` from `vector<InstrumentBus*>`
   to `map<int, InstrumentBus*>`, eliminating NULL checks and mActiveInstBusCount.
5. Added `RTcmix::getInstBus(int busID)` static convenience method, eliminating
   repeated manager lookups and NULL checks in intraverse and rtgetin.
6. Removed legacy fallback path in rtgetin (every aux bus with consumers has
   an InstrumentBus).
7. Removed `mLastPreparedAt` member (redundant with `mLastAdvancedBufStart`
   for dual-phase idempotency).
8. Moved `cycleCount` variable inside `#ifdef IBUG` blocks.

## Scheduling Logic Unification (2026-02-23)

Unified ~40 lines of duplicate scheduling logic between the intraverse push
path and `InstrumentBus::runWriterCycle()` pull path into two shared static
methods on InstrumentBus:

1. **`popAndPrepareWriters`** (static, public): Pops instruments from a single
   queue, computes timing (offset, chunksamps, output_offset), sets values on
   each instrument.  Does NOT execute — the caller chooses parallel (TaskManager)
   or sequential execution.  Parameterized by:
   - `busq`: queue index (explicit, from caller's loop)
   - `timelineOrigin`: `bufStartSamp` (push) or `mFramesProduced` (pull)
   - `bufEnd`: `bufEndSamp` (push) or `localBufEnd` (pull)
   - `writeStart`: `instBusWriteStart` (push) or ring buffer position (pull)
   - `maxFrames`: `frameCount` / `mBufsamps` (same value)

2. **`requeueOrUnref`** (static, public): Re-queues instruments that haven't
   finished, unrefs those that have.  Parameterized by:
   - `busq`: explicit queue index for requeue (always >= 0)
   - `qStatus`: phase guard for unref — `UNKNOWN` means always unref if
     finished (pull path); a specific `IBusClass` means only unref when
     `qStatus == iBus->Class()` (push path, ensuring all buses have played)

3. **`runWriterCycle`** rewritten to call per-queue: iterates TO_AUX and
   AUX_TO_AUX queues, calling `popAndPrepareWriters` + sequential exec +
   `requeueOrUnref` for each.

4. **intraverse MULTI_THREAD path**: Pop-while-loop and requeue-for-loop
   replaced with calls to the static helpers.  Execution still uses TaskManager.

Net result: -80 lines, single source of truth for scheduling logic.  Queue
sort and `allQSize` tracking remain caller responsibilities.

## Design Decisions

1. **Hybrid push/pull** -- intraverse phased execution preserved (not replaced
   by pure pull).  The push path handles the common 1:1 case with zero overhead.
   The pull path handles overflow for rate-changing instruments.

2. **Queue-based production** -- `runWriterCycle()` pops from rtQueue instead
   of maintaining a separate writer list.  rtQueue handles all timing edge cases.

3. **Ring buffer positions derived** -- No stored write cursor.  Write position
   is always `mFramesProduced % mBufferSize`.  Only monotonic counters stored.

4. **Sequential execution in pull path** -- `runWriterCycle` executes writers
   sequentially (not via TaskManager) because it runs inside a TaskThread.
   Nested `startAndWait` with participatory waiting would cause unbounded
   recursion.

5. **Idempotent per-cycle methods** -- `advanceProduction`, `hasRoomForProduction`,
   and `prepareForIntraverseWrite` take `FRAMETYPE currentBufStart` to detect
   dual-phase visits.  No conditionals at call sites in intraverse.

6. **All timing in global timeline** -- Instrument scheduling (endsamp,
   ichunkstart, re-queue) stays in the global `bufStartSamp` timeline.  Only
   `output_offset` uses the ring buffer write position.  An earlier attempt to
   remap timing into a per-bus timeline caused instruments to never finish.

7. **Static scheduling helpers** -- `popAndPrepareWriters` and `requeueOrUnref`
   are static methods because they access only `RTcmix::rtQueue` and instrument
   state, not InstrumentBus members.  This lets intraverse call them for all
   bus types (including non-InstrumentBus TO_OUT buses), eliminating all
   scheduling duplication.

## References

- Testing procedures: `project_testing.md`
- Architecture doc: `project_arch.md`
- Project plan: `project_plan.md`
- InstrumentBus implementation: `src/rtcmix/InstrumentBus.cpp`
- InstrumentBusManager: `src/rtcmix/InstrumentBusManager.cpp`
- Push path: `src/rtcmix/intraverse.cpp`
- Pull entry point: `src/rtcmix/rtgetin.cpp`
- Buffer clearing: `src/rtcmix/buffers.cpp`
- Instrument lifecycle: `src/rtcmix/Instrument.cpp`
- Per-thread accumulation: `src/rtcmix/bus_config.cpp`
- Task management: `src/rtcmix/TaskManager.h`
