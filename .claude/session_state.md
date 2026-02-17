# Session State: InstrumentBus Pull Model Conversion
# Last updated: 2026-02-15
# Branch: Signal-Pull-Model
# Latest commit: 93e06668

## Implementation Status

All 6 plan steps are COMPLETE (code changes applied). Currently debugging a
test failure in rate-changing (TRANS) chains.

### Completed Code Changes

1. **InstrumentBus.h/.cpp** — Fully rewritten. runWriterCycle pops from rtQueue,
   computes timing, executes, re-queues. pullFrames is the demand loop.

2. **InstrumentBusManager.h/.cpp** — Removed addWriter/removeWriter methods.

3. **intraverse.cpp** — 6 edits applied:
   - Removed writer registration from heap-pop loop
   - Removed deferredInsts declaration
   - Added `qStatus != TO_OUT` skip for non-TO_OUT buses
   - Simplified pop loop to unconditional exec (no defer path)
   - Removed deferredInsts lifecycle processing block
   - Removed `#include <utility>`

4. **rtgetin.cpp** — Already correct (direct pullFrames call). No changes needed.

5. **Instrument.cpp** — Removed removeWriter from destructor, removed
   `#include "InstrumentBus.h"`, updated comment.

6. **Debug output** — Changed `#define IBUG` to `#undef IBUG` in rtgetin.cpp
   and Instrument.cpp. InstrumentBus.cpp and InstrumentBusManager.cpp already
   had `#undef` by default.

### Current Debug State (TEMPORARY — revert before committing)

Several files have temporary debug printfs added during investigation:
- `InstrumentBus.cpp`: IBUG and BBUG `#define`d, before/after mixToBus sample prints
- `Instrument.cpp`: printf in exec() showing needs_to_run and outbuf values
- `bus_config.cpp`: printf in addToBus showing type/bus/offset/src values

These ALL need to be reverted to clean state before committing.

## Test Results (Post-Refactor)

| Test | Expected | Actual | Status |
|------|----------|--------|--------|
| instbus_basic_test | ~10000 ch0 | 9999.999 | PASS |
| instbus_deep_chain_test | ~11809 | 11809.797 | PASS |
| instbus_multi_writer_test | ~10000 ch0 | 9999.599 | PASS |
| instbus_chain_test (2 TRANS) | nonzero | 0.302 | FAIL |

### Additional TRANS chain tests:

| Configuration | Result | Status |
|---------------|--------|--------|
| 1 TRANS, no shift (transp=0.0) → output | 5000/ch | PASS |
| 1 TRANS, octave up (transp=1.0) → output | 5000/ch | PASS |
| WT→TRANS(1.0)→STEREO (3-stage with shift) | 5000/ch | PASS |
| 2 TRANS both unshifted (0.0, 0.0) | 5000/ch | PASS |
| 2 TRANS first shifted (1.0), second unshifted (0.0) | 5000/ch | PASS |
| 2 TRANS first unshifted (0.0), second shifted (1.0) | 5000/ch | PASS |
| 2 TRANS both tiny shift (0.01, 0.01) | 5000/ch | PASS |
| 2 TRANS both shift 0.5 | 1.7 | FAIL |
| 2 TRANS both shift 1.498/1.335 | 0.3 | FAIL |

## Active Bug: 2-TRANS Chain With Non-Integer-Multiple Transposition

### What We Know

The failure depends on the SPECIFIC transposition value:
- **transp=0.0** (ratio=1.0): WORKS — 1:1, TRANS consumes exactly 512 per 512 output
- **transp=1.0** (ratio=2.0): WORKS — TRANS consumes exactly 1024 per 512 output (integer multiple of 512)
- **transp=0.01** (ratio≈1.007): WORKS — very close to 1:1
- **transp=0.5** (ratio≈1.414): FAILS — TRANS consumes ~724 per 512 output (NOT integer multiple)
- **transp=1.498** (ratio≈2.82): FAILS — similar non-integer-multiple issue

Single TRANS with ANY ratio works fine. The bug only manifests with 2+ TRANS stages.

### Key Debug Finding

The FIRST exec of TRANS1 (writing to bus 1) produces outbuf[0]=0.0 (all zeros).
Subsequent execs produce real data. This is true even though needs_to_run=1 and
run() does execute on the first call.

TRANS1 DOES pull from bus 0 (verified — bus 0 produces real WAVETABLE data).
But TRANS1's output is all zeros on first exec.

### Theories NOT YET Investigated

1. The zero first-exec might also occur in the WORKING cases (transp=0.0, 1.0)
   but compensated for somehow. Need to verify.

2. TRANS::run() first call: getframe starts uninitialized? Or the interpolation
   produces zeros because oldersig/oldsig/newsig are all zero on first entry?
   Need to check TRANS member initialization.

3. The user's hint: "fix the order of operations regarding writing, adding,
   reading, and clearing." The issue may be about when clearRegion happens
   relative to when data is read. With non-integer-multiple ratios, TRANS
   leaves partial data in the buffer that gets cleared on the next cycle.

### User's Key Hint

"Do not solve this by changing the aux buffer sizes. Fix the order of operations
regarding writing, adding, reading, and clearing."

This suggests the fix is in runWriterCycle or pullFrames — changing WHEN we clear,
write, add (mixToBus), or read relative to each other. NOT buffer size changes.

## Resolved Design Decisions (from previous session)

### 1. No global bufEndSamp/bufStartSamp
Output InstrumentBus mFramesProduced is the master clock.

### 2. Ring buffer positions are derived
mWritePosition redundant — use mFramesProduced % mBufferSize.

### 3. Instruments re-enqueue/dequeue through rtQueue
Each demand loop iteration: pop, compute offset, exec, re-queue.

### 4. InstrumentBus owns the production loop
pullFrames contains demand-driven production. All state via RTcmix statics.

### 5. Parameter timing (known characteristic of new functionality)
Real-time params upstream of rate-changers update at output buffer rate.

### 6. Heap pop V1 limitation (acceptable)
One-buffer delay for new starts on upstream buses in rate-changing chains.

## How to Build and Test

```bash
cd /opt/local/src/RTcmix.git
make -j4 && make install
/opt/local/src/RTcmix.git/bin/CMIX < test/suite/instbus_basic_test.sco
```
