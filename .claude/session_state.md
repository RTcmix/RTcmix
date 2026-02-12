# Session State: InstrumentBus Pull Model Conversion
# Last updated: 2026-02-12
# Branch: Signal-Pull-Model
# Latest commit: 93e06668

## What Was Just Done

Committed all work-in-progress as `93e06668` on the `Signal-Pull-Model` branch.
The commit includes InstrumentBus infrastructure, intraverse modifications for
deferred execution, TaskManager nested parallelism, and 17 test scores.

## Where We Left Off

We were in the middle of an **architectural design discussion** about how
`runWriterCycle()` should relate to intraverse's queue-based scheduling.

The conversation reached a consensus that **Option C** is the best path forward:
- InstrumentBus becomes a thin ring buffer tracker (no execution logic)
- All queue processing, instrument execution, and lifecycle management stays in intraverse
- A new `processAuxBus(busID, framesNeeded)` function in intraverse handles the
  demand-driven loop (run writers until enough frames are available)

### The Core Problem Being Solved

`runWriterCycle()` currently operates autonomously: it maintains its own writer list,
calls `setchunk()` and `set_output_offset()` on instruments, and runs them via
TaskManager. This bypasses intraverse's queue-based scheduling (rtQueue, bufEndSamp,
rtQchunkStart, offset, chunksamps) and causes:

1. **Timing errors**: Writers that haven't reached their start time within a buffer
   cycle get run anyway (staggered entry test fails)
2. **Queue desync**: If pullFrames runs writers multiple times (elastic case for TRANS),
   the instruments advance further than intraverse's re-queue logic expects, because
   re-queue uses `rtQchunkStart + chunksamps` but doesn't know about the extra runs

### The Design for Option C

```
InstrumentBus (thin):
  - mWritePosition, mFramesProduced
  - mConsumers map (per-consumer read cursors)
  - framesAvailable(consumer)
  - advanceWriteCursor(frames)
  - getReadPosition(consumer, frames) -> returns pos, advances cursor
  - clearRegion(pos, frames)
  - NO runWriterCycle, NO mWriters, NO execution logic

intraverse processAuxBus(busID, framesNeeded):
  InstrumentBus* ib = instBusMgr->getInstBus(busID);
  while (ib->framesAvailable(consumer) < framesNeeded) {
      ib->clearRegion(ib->getWritePosition(), bufsamps);
      // Normal queue processing from intraverse (same code as existing):
      //   pop instruments from rtQueue[busq]
      //   compute offset, chunksamps per instrument
      //   setchunk, set_output_offset
      //   addTask / exec
      //   waitForTasks / mixToBus
      // Normal lifecycle:
      //   re-queue (push at rtQchunkStart + chunksamps) or unref
      ib->advanceWriteCursor(bufsamps);
  }

Pull chain trigger:
  TO_OUT instrument exec -> run() -> rtgetin() -> pullFrames()
  -> pullFrames sees framesAvailable < requested
  -> calls back to processAuxBus (mechanism TBD: callback? direct call?)
  -> processAuxBus runs upstream writers via normal queue processing
```

Key detail: `bufEndSamp` is a file-static global in intraverse.cpp, shared across
all buses. `processAuxBus` needs a per-bus timeline — `InstrumentBus::mFramesProduced`
serves this purpose. The per-bus "bufEndSamp equivalent" would be
`busStartSamp + mFramesProduced` or similar.

### Open Questions for Implementation

1. **Callback mechanism**: How does `pullFrames()` (called from rtgetin, which is
   called from inside an instrument's `run()`) get back to intraverse's
   `processAuxBus()`? Options:
   - Function pointer/callback stored on InstrumentBus
   - InstrumentBus calls a static method on RTcmix
   - pullFrames is removed; rtgetin calls processAuxBus directly

2. **Per-bus bufEndSamp**: Each bus needs its own notion of "how far we've produced"
   to gate the `rtQchunkStart < bufEndSamp` check in the queue processing loop.
   This replaces the global `bufEndSamp` for upstream buses.

3. **Re-entrant queue processing**: processAuxBus runs an instrument that itself
   calls rtgetin, which calls processAuxBus for a further-upstream bus. This is
   recursive. The rtQueue for each bus is independent, so this should be safe,
   but needs verification.

4. **clear_aux_buffers() at end of inTraverse**: Currently clears ALL aux buffers
   every cycle. With ring buffer management, this conflicts with InstrumentBus's
   cursor tracking. Needs to be disabled for buses managed by InstrumentBus.

## Test Results Summary

| Test | Expected Peak | Actual Peak | Status |
|------|--------------|-------------|--------|
| instbus_basic_test | ~4000 | ~4000 | PASS |
| instbus_chain_test | ~4000 | ~4000 | PASS |
| instbus_multi_writer_test | ~8000 | ~8000 | PASS |
| instbus_multilevel_merge_test | ~12000 | ~12000 | PASS |
| instbus_multi_writer_cascade_test | ~12000 | ~12000 | PASS |
| instbus_deep_chain_test | ~11809 | ~11809 | PASS |
| instbus_asymmetric_tree_test | ~12000 | ~20000 | FAIL |
| instbus_staggered_entry_test | ~12000 | ~4267 | FAIL (was crash, now wrong amplitude) |
| instbus_dual_mono_to_stereo_test | both channels | both channels | PASS (after STEREO fix) |

Other test scores (diamond, parallel_merge, stereo_merge, quad, complex_quad,
dual_mono_separate, trans, trans_down) have not been systematically verified with
peak amplitude checks.

## Files with Verbose Debug Output (disable before testing)

These files have IBUG or DBUG `#define`d (not `#undef`d), causing thousands of
lines of output per intraverse cycle:

- `src/rtcmix/InstrumentBus.cpp` lines 27-28: `#define IBUG` and `#define DBUG`
- `src/rtcmix/Instrument.cpp` line 29: `#define IBUG`
- `src/rtcmix/rtgetin.cpp` line 26: `#define IBUG`

To run tests without debug flood, either:
1. Change these to `#undef` before building
2. Pipe test output through `grep` for specific patterns (e.g., `grep "peak"`)

## How to Build and Test

```bash
cd /opt/local/src/RTcmix.git
make -j4          # builds everything including instruments
# Run a test:
CMIX /opt/local/src/RTcmix.git/test/suite/instbus_basic_test.sco
# Check peak amplitude of output:
sfpeak /opt/local/src/RTcmix.git/test/suite/*.wav  # or wherever output goes
```

Note: The CMIX binary is at `/opt/local/bin/CMIX` (or wherever make install puts it).

## Next Step

Implement Option C: refactor InstrumentBus to be a thin buffer tracker and move
all execution logic into intraverse's new `processAuxBus()` function. This involves:

1. Strip InstrumentBus of `runWriterCycle()`, `mWriters`, `mActiveWriters`, `mTaskManager`
2. Add `advanceWriteCursor()` and modify `pullFrames()` to NOT trigger production
   (just return current availability or block)
3. Create `processAuxBus(busID, framesNeeded)` in intraverse that contains the
   queue processing loop with the demand-driven outer while loop
4. Wire up the callback from pullFrames/rtgetin to processAuxBus
5. Handle per-bus bufEndSamp tracking
6. Test with existing passing test scores first, then fix failing ones