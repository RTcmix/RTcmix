/* instbus_staggered_entry_test.sco
 *
 * Test: Writers to the same bus with staggered start times
 *
 * Routing:
 *   WAVETABLE(440Hz, 4000, t=0-3s)  -> aux 0
 *   WAVETABLE(440Hz, 4000, t=1-3s)  -> aux 0  (starts 1s late)
 *   WAVETABLE(440Hz, 4000, t=2-3s)  -> aux 0  (starts 2s late)
 *     -> MIX(aux 0) -> out 0-1
 *
 * 440Hz at 44100 SR: exactly 440 cycles/sec, so at t=1s and t=2s
 * a new WAVETABLE starting at phase 0 is in phase with the running one.
 *
 * Tests: Writer registration at different times, InstrumentBus handling
 *        writers that join mid-stream
 *
 * Expected peak: ~12000 (3 x 4000, all in phase during 2-3s window)
 *   If staggered join fails: ~4000 (only first writer)
 *   If all sources fail: 0
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")

makegen(1, 24, 1000, 0,0, 0.05,1, 0.95,1, 1,0)
makegen(2, 10, 1000, 1)

amp = 4000

/* All three write to aux 0, staggered starts */
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, 3, amp, 440)
WAVETABLE(1, 2, amp, 440)
WAVETABLE(2, 1, amp, 440)

/* Output */
bus_config("MIX", "aux 0 in", "out 0-1")
MIX(0, 0, 3, 1, 0, 1)

print("instbus_staggered_entry_test: 3 writers join aux 0 at t=0,1,2")
print("Expected peak: ~12000 (3 x 4000, all in phase)")