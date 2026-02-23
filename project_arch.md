# RTcmix InstrumentBus Pull Model: Architecture

This document describes the architecture of the InstrumentBus pull model as
implemented.  It covers the runtime data flow, class structure, threading
model, and the dual-path execution strategy that integrates pull-based
production with the existing phased intraverse loop.

---

## 1. System Overview

```
                          ┌──────────────────────────┐
                          │      AudioDevice         │
                          │   (requests RTBUFSAMPS   │
                          │    frames per callback)  │
                          └────────────┬─────────────┘
                                       │
                              inTraverse() callback
                                       │
                    ┌──────────────────────────────────────┐
                    │                                      │
          ┌────────┴────────┐                    ┌────────┴────────┐
          │   Phased Push   │                    │   Demand Pull   │
          │  (intraverse)   │                    │ (InstrumentBus) │
          │                 │                    │                 │
          │ TO_AUX phase    │  ──── overflow ──> │ pullFrames()    │
          │ AUX_TO_AUX phase│  <── production ── │ runWriterCycle()│
          │ TO_OUT phase    │                    │                 │
          └─────────────────┘                    └─────────────────┘
```

The system uses a **hybrid push/pull** model:

- **Push path (intraverse):** The main audio callback processes buses in phase
  order (TO_AUX, AUX_TO_AUX, TO_OUT). For each bus, instruments are popped
  from `rtQueue`, executed via `TaskManager`, and re-queued. This is the
  original RTcmix execution model, preserved intact.

- **Pull path (InstrumentBus):** When a downstream instrument calls `rtgetin()`
  and the input comes from an aux bus managed by an InstrumentBus, `pullFrames()`
  is called. If insufficient frames are available, `runWriterCycle()` pops
  instruments from `rtQueue` and executes them on demand.

The push path handles the common case (1:1 frame ratio). The pull path handles
the overflow case (rate-changing instruments that need more input than one
push cycle provides).

---

## 2. Class Structure

### 2.1 InstrumentBus

One per aux bus.  Wraps `RTcmix::aux_buffer[busID]` as a ring buffer.

```
InstrumentBus
├── mBusID               int            Which aux bus
├── mBufsamps            int            Frames per production chunk (RTBUFSAMPS)
├── mBufferSize          int            Ring buffer size (bufsamps * MULTIPLIER)
├── mFramesProduced      FRAMETYPE      Monotonic production counter
├── mConsumers           map<Inst*,CS>  Per-consumer read cursors
├── mPullLock            Lockable       Serializes pullFrames per bus
├── mLastPreparedAt      FRAMETYPE      Dedup clearRegion calls
├── mLastAdvancedBufStart FRAMETYPE     Dedup dual-phase production
│
├── pullFrames(consumer, frames) → readPos     [public, locked]
├── addConsumer(inst)                          [public, locked]
├── removeConsumer(inst)                       [public, locked]
├── hasRoomForProduction(bufStartSamp) → bool  [public, const]
├── prepareForIntraverseWrite(bufStartSamp) → writePos  [public]
├── advanceProduction(frames, bufStartSamp)    [public]
├── framesAvailable(consumer) → int            [public, const]
├── getReadPosition(consumer, frames) → readPos [public]
│
└── runWriterCycle()                           [private]
    clearRegion(start, frames)                 [private]
```

**No separate buffer allocation.** InstrumentBus reads and writes
`RTcmix::aux_buffer[mBusID]` directly.  The ring buffer position is derived:
`writePos = mFramesProduced % mBufferSize`.  No stored write cursor.

### 2.2 InstrumentBusManager

Singleton owned by RTcmix.  Manages all InstrumentBus objects.

```
InstrumentBusManager
├── mInstBuses    vector<InstrumentBus*>   Indexed by bus ID (NULL = unmanaged)
├── mBufsamps     int                      RTBUFSAMPS
│
├── getOrCreateInstBus(busID) → InstrumentBus*   Lazy creation
├── getInstBus(busID) → InstrumentBus*           NULL if not created
├── addConsumer(busID, inst)                      Creates bus if needed
├── removeConsumer(inst)                          Iterates all buses
└── advanceAllProduction(frames, bufStartSamp)    Advances all buses
```

### 2.3 ConsumerState

```
struct ConsumerState {
    FRAMETYPE framesConsumed;   // Monotonic counter
};
```

Both `mFramesProduced` and `framesConsumed` are monotonic absolute frame
counters.  Ring buffer positions are always derived via `% mBufferSize`.
Available frames = `mFramesProduced - framesConsumed`.

---

## 3. Data Flow

### 3.1 Normal Case: 1:1 Frame Ratio

```
inTraverse cycle (bufStartSamp = 0, RTBUFSAMPS = 512):

  TO_AUX phase:
    bus 0: pop WAVETABLE from rtQueue[0]
           prepareForIntraverseWrite(0) → writeStart=0, clears [0,512)
           TaskManager: exec WAVETABLE → writes to aux_buffer[0][0..511]
           waitForTasks + mixToBus
           advanceProduction(512, 0) → mFramesProduced = 512

  AUX_TO_AUX phase:
    bus 1: pop MIX from rtQueue[busCount+1]
           prepareForIntraverseWrite(0) → writeStart=0, clears [0,512)
           TaskManager: exec MIX
             MIX::run() → rtgetin() → pullFrames(this, 512)
               framesAvailable = 512 (intraverse already produced)
               returns readPos=0
             MIX reads aux_buffer[0][0..511], writes aux_buffer[1][0..511]
           waitForTasks + mixToBus
           advanceProduction(512, 0) → mFramesProduced = 512

  TO_OUT phase:
    bus 0: pop STEREO from rtQueue[2*busCount]
           TaskManager: exec STEREO
             STEREO::run() → rtgetin() → pullFrames(this, 512)
               framesAvailable = 512
               returns readPos=0
             STEREO reads aux_buffer[1][0..511], writes out_buffer[0..1]
           waitForTasks + mixToBus

  rtsendsamps → hardware
  bufStartSamp += 512
```

In the 1:1 case, `pullFrames()` never calls `runWriterCycle()` because
intraverse already produced all frames.  The pull path is a zero-cost
lookup.

### 3.2 Rate-Changing Case: TRANS Down (2:1 Consumption)

TRANS transposes down by one octave: reads 256 input frames to produce 512
output frames.  Every other intraverse cycle, TRANS has leftover input and
does not need more.

```
Cycle 0 (bufStartSamp = 0):
  TO_AUX bus 0: WAVETABLE produces 512 frames
    advanceProduction(512, 0) → mFramesProduced = 512

  AUX_TO_AUX bus 1: TRANS executes
    TRANS::run() → rtgetin() → pullFrames(this, 256)
      framesAvailable = 512 (more than enough)
      returns readPos=0, consumer advances to framesConsumed=256

  TO_OUT: STEREO reads from bus 1, outputs to hardware

Cycle 1 (bufStartSamp = 512):
  TO_AUX bus 0: hasRoomForProduction(512)?
    avail = 512 - 256 = 256.  256 + 512 > 512 (bufferSize).  → false
    SKIP.  WAVETABLE stays on rtQueue.

  AUX_TO_AUX bus 1: TRANS executes
    TRANS::run() → rtgetin() → pullFrames(this, 256)
      framesAvailable = 512 - 256 = 256 (exactly enough from cycle 0)
      returns readPos=256, consumer advances to framesConsumed=512

  TO_OUT: STEREO reads from bus 1

Cycle 2 (bufStartSamp = 1024):
  TO_AUX bus 0: hasRoomForProduction(1024)?
    avail = 512 - 512 = 0.  0 + 512 ≤ 512.  → true
    WAVETABLE produces 512 more frames.  mFramesProduced = 1024.
    ...
```

The production guard (`hasRoomForProduction`) prevents intraverse from
overwriting unconsumed data.  WAVETABLE stays on `rtQueue` and executes on
the next cycle when the consumer has caught up.

### 3.3 Rate-Changing Case: TRANS Up (Pull-Driven Production)

TRANS transposes up by one octave: reads 1024 input frames to produce 512
output frames.  The push path provides 512; `pullFrames` produces 512 more.

```
Cycle 0 (bufStartSamp = 0):
  TO_AUX bus 0: WAVETABLE produces 512 frames
    advanceProduction(512, 0) → mFramesProduced = 512

  AUX_TO_AUX bus 1: TRANS executes
    TRANS::run() → rtgetin() → pullFrames(this, 1024)
      framesAvailable = 512 (not enough)
      runWriterCycle():
        pops WAVETABLE from rtQueue[0]
        exec WAVETABLE → produces 512 more → mFramesProduced = 1024
        re-queues WAVETABLE at rtQchunkStart + 512
      framesAvailable = 1024 (enough)
      returns readPos=0, consumer advances to framesConsumed=1024

  TO_OUT: STEREO reads from bus 1
```

`runWriterCycle()` is the pull fallback.  It mirrors intraverse's per-bus
pattern: pop from `rtQueue`, compute timing, exec, mixToBus, re-queue.
Writers execute sequentially (not via TaskManager) to avoid nested
parallelism issues.

---

## 4. Dual-Phase Idempotency

A single bus can appear in both the TO_AUX and AUX_TO_AUX playlists (e.g.,
bus 6 has WAVETABLE writers in TO_AUX and MIX writers in AUX_TO_AUX).
Three methods use `mLastAdvancedBufStart` to make dual-phase visits safe:

| Method | First phase visit | Second phase visit (same cycle) |
|--------|-------------------|-------------------------------|
| `hasRoomForProduction(bss)` | Normal consumer check | Returns `true` (allow accumulation) |
| `prepareForIntraverseWrite(bss)` | Clears region, returns writePos | Returns previous writePos, no clear |
| `advanceProduction(n, bss)` | Advances `mFramesProduced` | No-op |

This ensures that both TO_AUX and AUX_TO_AUX writers accumulate into the
same buffer region, and the production counter advances exactly once.

---

## 5. Consumer Lifecycle

### 5.1 Registration

```
Instrument::set_bus_config()          ← called at parse time (Minc thread)
  └─ InstrumentBusManager::addConsumer(busID, this)
       └─ InstrumentBus::addConsumer(inst)     [AutoLock]
            └─ mConsumers[inst] = { framesConsumed: max(mFramesProduced, bufStartSamp) }
```

Consumers are registered at parse time so they exist before the pull chain
runs.  The initial `framesConsumed` is set to the later of `mFramesProduced`
and `bufStartSamp` to handle both idle and active buses correctly.

### 5.2 Removal

```
Instrument::~Instrument()
  └─ InstrumentBusManager::removeConsumer(this)
       └─ for each bus: InstrumentBus::removeConsumer(inst)  [AutoLock]
            └─ mConsumers.erase(inst)
```

Dead consumers are removed from all buses on destruction.  Without this,
`hasRoomForProduction()` would see a stale `framesConsumed` value from the
dead consumer and block production forever.

### 5.3 Writers

Writers are **not explicitly registered** with InstrumentBus.  Instead,
both intraverse (push path) and `runWriterCycle()` (pull path) pop writers
from `rtQueue` at execution time.  The queue handles all timing, lifecycle
(re-queue/unref), and ordering.

---

## 6. Ring Buffer Design

```
aux_buffer[busID]:  [frame0][frame1][frame2]...[frame(N-1)]
                     ↑                          ↑
                     consumer read position     write position
                     (framesConsumed % N)       (mFramesProduced % N)

N = mBufferSize = RTBUFSAMPS * INSTBUS_BUFFER_MULTIPLIER
```

| Property | Value |
|----------|-------|
| Storage | `RTcmix::aux_buffer[busID]` (no separate allocation) |
| Size | `RTBUFSAMPS * INSTBUS_BUFFER_MULTIPLIER` frames |
| Write position | Derived: `mFramesProduced % mBufferSize` |
| Read position | Derived: `framesConsumed % mBufferSize` |
| Available frames | `mFramesProduced - framesConsumed` |
| Overwrite guard | `hasRoomForProduction`: `avail + bufsamps ≤ bufferSize` |

With `INSTBUS_BUFFER_MULTIPLIER = 1` (default), the ring buffer equals
`RTBUFSAMPS`.  Each cycle fully consumes the previous cycle's data (1:1
instruments) or the production guard defers writing until the consumer
catches up (rate-changing instruments).

### Clearing Strategy

`clear_aux_buffers()` (called at end of every `inTraverse` cycle) **skips**
InstrumentBus-managed buses.  Instead, clearing happens in two places:

1. **`prepareForIntraverseWrite()`** -- clears the write region before
   intraverse executes writers for this bus.

2. **`runWriterCycle()`** -- clears the write region before pull-driven
   writers execute.

Both use `clearRegion()` which handles wrap-around.

---

## 7. Threading Model

### 7.1 Push Path (intraverse, MULTI_THREAD)

```
Main thread (audio callback):
  for each bus in TO_AUX playlist:
    for each instrument on rtQueue[bus]:
      taskManager->addTask(inst->exec)     ← parallel
    taskManager->waitForTasks(instruments)  ← barrier
    RTcmix::mixToBus()                     ← merge per-thread vectors
    instBus->advanceProduction(...)
```

Multiple instruments writing to the same bus execute in parallel via
`TaskManager`.  Per-thread accumulation vectors (`mixVectors[]`) avoid
contention.  `mixToBus()` merges them after the barrier.

### 7.2 Pull Path (runWriterCycle)

```
TaskThread (via exec → run → rtgetin → pullFrames):
  AutoLock(mPullLock)                      ← serialize per bus
  while (framesAvailable < requested):
    runWriterCycle():
      for each instrument on rtQueue[bus]:
        inst->exec(BUS_AUX_OUT, busID)     ← sequential, same thread
      RTcmix::mixToBus()                   ← merge (still needed in MT mode)
```

Writers execute **sequentially** in `runWriterCycle()`, not via
`TaskManager`.  This is because `runWriterCycle` is called from a
`TaskThread` (the call chain is: `TaskThread` runs `Instrument::exec` →
`run` → `rtgetin` → `pullFrames` → `runWriterCycle`).  Using `TaskManager`
from within a `TaskThread` would cause nested `startAndWait`, where
participatory waiting could steal unrelated tasks and cause unbounded
recursion.

`mPullLock` serializes production per bus.  Multiple consumers calling
`pullFrames` concurrently (from different `TaskThreads`) are serialized
because `rtQueue` (a `std::vector`) is not thread-safe.

### 7.3 Single-Threaded Path

Both intraverse and `runWriterCycle` execute instruments directly via
`inst->exec()`.  No `TaskManager`, no `mixToBus()` (accumulation goes
directly into `aux_buffer`).  The same control flow applies; the threading
primitives compile away.

---

## 8. Timeline and Timing

### 8.1 Two Timelines

| Timeline | Scope | Counter | Used for |
|----------|-------|---------|----------|
| Global | System-wide | `bufStartSamp` | Instrument scheduling (endsamp, ichunkstart, re-queue) |
| Per-bus | InstrumentBus | `mFramesProduced` | Ring buffer position, frames available |

**Key rule:** All instrument timing (offsets, chunksamps, re-queue decisions)
stays in the global `bufStartSamp` timeline.  Only `output_offset` (where
an instrument writes in `aux_buffer`) uses the InstrumentBus ring buffer
position.  This prevents the timeline divergence that would occur if
instruments were scheduled in a local bus timeline (where endsamp in global
time could never be reached in local time for rate-changing chains).

### 8.2 Fast-Forward

When a bus has been idle and a consumer first calls `pullFrames`,
`mFramesProduced` may be behind `framesConsumed` (which was set to
`bufStartSamp` in `addConsumer`).  `pullFrames` fast-forwards
`mFramesProduced` to match, so `runWriterCycle` pops instruments at the
correct time.  No phantom frames are created.

---

## 9. Integration Points

### 9.1 rtgetin.cpp

The entry point for pull-based input:

```cpp
if (fdindex == NO_DEVICE_FDINDEX) {     // Input from aux buses
    InstrumentBus *instBus = mgr->getInstBus(auxin[0]);
    if (instBus != NULL) {
        // Pull path: trigger production, get read position
        for (each auxin channel)
            readPos = instBus->pullFrames(this, frames);
        RTcmix::readFromAuxBus(inarr, ..., readPos);
    } else {
        // Legacy path: read directly from aux_buffer at output_offset
        RTcmix::readFromAuxBus(inarr, ..., output_offset);
    }
}
```

If an InstrumentBus exists for the input bus, the pull path is used.
Otherwise the legacy direct-read path applies (for buses not managed by
InstrumentBus).

### 9.2 buffers.cpp

`clear_aux_buffers()` skips InstrumentBus-managed buses:

```cpp
for (i = 0; i < busCount; i++) {
    if (mgr && mgr->getInstBus(i))
        continue;           // InstrumentBus manages its own clearing
    // ... zero aux_buffer[i] ...
}
```

### 9.3 Instrument.cpp

- **Constructor:** Standard initialization (unchanged).
- **`set_bus_config()`:** Registers as consumer of each `auxin` bus via
  `instBusMgr->addConsumer(auxin[i], this)`.
- **Destructor:** Calls `instBusMgr->removeConsumer(this)` before releasing
  `_busSlot`, ensuring dead consumers don't block production.

### 9.4 intraverse.cpp

Three additions to the existing phased loop:

1. **Production guard** (before executing a bus):
   ```cpp
   if (instBus && !instBus->hasRoomForProduction(bufStartSamp))
       continue;   // instruments stay on rtQueue
   ```

2. **Write position** (before popping instruments):
   ```cpp
   instBusWriteStart = instBus->prepareForIntraverseWrite(bufStartSamp);
   Iptr->set_output_offset(instBusWriteStart + offset);
   ```

3. **Production advance** (after waitForTasks + mixToBus):
   ```cpp
   instBus->advanceProduction(frameCount, bufStartSamp);
   ```

---

## 10. File Map

| File | Role |
|------|------|
| `InstrumentBus.h` | Class declaration, ring buffer constants |
| `InstrumentBus.cpp` | Pull/production logic, ring buffer operations |
| `InstrumentBusManager.h` | Manager declaration |
| `InstrumentBusManager.cpp` | Bus creation, consumer routing |
| `intraverse.cpp` | Push-path integration (production guard, write pos, advance) |
| `rtgetin.cpp` | Pull-path entry point (pullFrames in rtgetin) |
| `buffers.cpp` | Clearing exclusion for managed buses |
| `Instrument.cpp` | Consumer registration (set_bus_config) and removal (destructor) |
| `RTcmix.h` | Static `instBusManager` member, `friend class InstrumentBus` |

---

## 11. Worked Example: WAVETABLE -> TRANS(+1oct) -> STEREO -> Output

```
Score:
  bus_config("WAVETABLE", "aux 0 out")
  bus_config("TRANS",     "aux 0 in", "aux 1 out")
  bus_config("STEREO",   "aux 1 in", "out 0-1")

  WAVETABLE(0, 2, 20000, 440)
  TRANS(0, 0, 2, 1, 1.0)         // +1 octave: reads 2x, outputs 1x
  STEREO(0, 0, 2, 1, 0.5)

Setup:
  set_bus_config("TRANS")  → addConsumer(bus 0, TRANS)    [InstrumentBus 0 created]
  set_bus_config("STEREO") → addConsumer(bus 1, STEREO)   [InstrumentBus 1 created]

Runtime (RTBUFSAMPS=512, cycle 0, bufStartSamp=0):

  Heap pop → WAVETABLE, TRANS, STEREO pushed to rtQueues

  TO_AUX phase, bus 0:
    hasRoomForProduction(0) → true (mFramesProduced=0, consumer framesConsumed=0)
    prepareForIntraverseWrite(0) → clears [0,512), returns 0
    Pop WAVETABLE, exec → writes 512 frames to aux_buffer[0][0..511]
    waitForTasks + mixToBus
    advanceProduction(512, 0) → mFramesProduced=512, mLastAdvancedBufStart=0

  AUX_TO_AUX phase, bus 1:
    hasRoomForProduction(0) → true
    prepareForIntraverseWrite(0) → clears [0,512), returns 0
    Pop TRANS, exec:
      TRANS::run() → rtgetin(in, this, 1024)   [needs 1024 frames for +1oct]
        pullFrames(TRANS, 1024):
          framesAvailable = 512 (not enough)
          runWriterCycle():
            pops WAVETABLE from rtQueue[0] at rtQchunkStart=512
            exec WAVETABLE → writes 512 to aux_buffer[0][0..511]
               (ring wraps: 512 % 512 = 0)
            mixToBus
            mFramesProduced = 1024
            re-queues WAVETABLE at 1024
          framesAvailable = 1024 (enough)
          getReadPosition → readPos=0, framesConsumed=1024
        readFromAuxBus(inarr, ..., readPos=0)
      TRANS reads 1024 from aux_buffer[0], outputs 512 to aux_buffer[1]
    waitForTasks + mixToBus
    advanceProduction(512, 0) → mFramesProduced=512

  TO_OUT phase, bus 0:
    Pop STEREO, exec:
      STEREO::run() → rtgetin(in, this, 512)
        pullFrames(STEREO, 512):
          framesAvailable = 512 (enough)
          getReadPosition → readPos=0, framesConsumed=512
        readFromAuxBus(inarr, ..., readPos=0)
      STEREO reads 512 from aux_buffer[1], writes to out_buffer[0..1]
    waitForTasks + mixToBus

  rtsendsamps → hardware
  bufStartSamp = 512, bufEndSamp = 1024
```

---

## 12. Known Limitations

1. **Diamond topology with TRANS up:** When two TRANS-up instruments share
   an upstream source, each pulls independently, causing the source to be
   consumed at the sum of their rates.  The source finishes early.

2. **Parameter timing upstream of rate changers:** Real-time parameters
   (MIDI, OSC) on instruments upstream of a rate changer update at the
   output buffer rate, not scaled by the rate change ratio.  This is a
   known characteristic, not a regression (TRANS/PVOC could not read from
   aux buses in the original push model).

3. **Heap pop V1 limitation:** Instruments starting beyond the output
   ceiling on upstream buses may be delayed one buffer cycle.
