# RTcmix Testing Procedures

## Build Rules

1. **Install before testing.** If CMIX or `librtcmix.dylib` are rebuilt,
   `make install` must be run before any verification testing. The test
   scores invoke the installed `CMIX` binary, not the build tree copy.

2. **Top-level install for Instrument changes.** If any code changes make
   a *structural* change to the `Instrument` class (member additions/removals,
   vtable changes, base class changes), run `make install` from the top level
   of the project so that all instrument shared libraries are relinked.

3. **Rebuild stresstest after header changes.** The `stresstest` binary in
   `test/suite/` is compiled against `RTcmix.h` and linked against
   `librtcmix.dylib`. The Makefile does not track header dependencies, so
   after any change to `RTcmix.h` or its transitive includes, manually
   clean and rebuild: `cd test/suite && rm -f stresstest.o stresstest &&
   make stresstest`. A stale binary will produce spurious "scheduler is
   behind the queue" warnings due to header/library mismatch.

4. **Verification suite.** After any code change, run all three of:
   - InstrumentBus tests
   - Stress test
   - Voice limit tests (system must support 14000+ voices)

   See commands below.

## Test Commands

All tests are run from `test/suite/`.

### InstrumentBus Tests (18 tests, 17 must pass)

```
cd test/suite && make instbus_tests
```

Tests bus routing topologies: basic chain, TRANS up/down, multi-writer,
deep chain, cascade, asymmetric tree, diamond, parallel merge, quad, stereo
merge, staggered entry, and TRANS reuse.

**Known failure:** `instbus_diamond_test` (choppy output, premature upstream
consumption with multiple TRANS consumers on shared bus). See CLAUDE.md.

### Stress Test

```
cd test/suite && make run_stresstest
```

Runs the embedded interactive stress test for 30 seconds (configurable via
`TESTLEN` in the Makefile). Exercises concurrent instrument creation and
execution under load.

### Voice Limit Tests

The system must support at least 14000 simultaneous voices without crashes
or hangs. Three scores test different routing topologies under high voice
counts:

```
cd test/suite
CMIX --voices=14000 < voicelimit_aux.sco
CMIX --voices=14000 < voicelimit_chain.sco
CMIX --voices=14000 < voicelimit_multi_aux.sco
```

- `voicelimit_aux.sco` — All voices through a single aux bus via MIX
- `voicelimit_chain.sco` — Chained instrument routing
- `voicelimit_multi_aux.sco` — Voices spread across multiple aux buses

All three must complete without assertion failures, crashes, or hangs.

## Quick Verification (copy-paste)

From the project root:

```
make install && cd test/suite && rm -f stresstest.o stresstest && make instbus_tests && make run_stresstest && CMIX --voices=14000 < voicelimit_aux.sco && CMIX --voices=14000 < voicelimit_chain.sco && CMIX --voices=14000 < voicelimit_multi_aux.sco
```

## Single-Threaded Verification

To verify that all code paths work in single-threaded mode (no `MULTI_THREAD`
define), rebuild only `src/rtcmix`:

```
cd src/rtcmix && make clean && cd ../.. && make MULTI_THREAD_SUPPORT=FALSE install
```

No other source needs recompilation — instrument shared libraries are unaffected.

Then run the same test suite:

```
cd test/suite && make instbus_tests && make run_stresstest
```

All InstrumentBus tests must pass (17/18, same diamond known failure).
**Exception:** The voice limit tests (`voicelimit_*.sco`) cannot play 14,000
voices in single-threaded mode and should be skipped.

**Note:** Both paths use the same shared helpers (`popAndPrepareWriters`,
`requeueOrUnref`). The only difference is sequential `exec()` calls in
single-threaded mode vs parallel TaskManager execution + `mixToBus()` in
multi-threaded mode.

To restore multi-threaded mode afterward:

```
cd src/rtcmix && make clean && cd ../.. && make install
```

## Debug Builds

Debug logging is controlled via `src/rtcmix/dbug.h`. Change `#undef` to
`#define` for the desired macro:

| Macro | Purpose |
|-------|---------|
| `ALLBUG` | All debug output |
| `BBUG` | Bus debugging (very verbose) |
| `DBUG` | General debug |
| `WBUG` | "Where we are" trace |
| `IBUG` | Instrument and InstrumentBus debugging |

**Warning:** Debug output is extremely verbose. Tests will appear to hang
if debug output is active and piped through `head`/`tail`.