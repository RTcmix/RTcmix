/* instbus_trans_reuse_test.sco
 *
 * Test: Long TRANS note with writers arriving and departing on its input bus.
 *
 * Routing:
 *   WAVETABLE -> aux 0 -> TRANS(unity) -> out 0-1
 *
 * Timeline:
 *   t=0-1:   WAVETABLE A (440Hz, amp 10000) writes to aux 0
 *   t=1-2:   gap — no writers on aux 0, TRANS reads silence
 *   t=2-3:   WAVETABLE B (880Hz, amp 10000) writes to aux 0
 *   t=3-4:   gap — no writers
 *   t=4-5:   WAVETABLE C (660Hz, amp 10000) writes to aux 0
 *
 * TRANS reads from aux 0 for the full 5 seconds at unity transposition.
 * Exercises: writer lifecycle on a continuously-pulled bus, correct
 * production when writers appear/disappear in the rtQueue.
 *
 * Expected: peak ~10000 on ch0 (from any of the WAVETABLE segments)
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")
load("TRANS")

bus_config("WAVETABLE", "aux 0 out")
bus_config("TRANS", "aux 0 in", "out 0-1")

/* Flat envelope */
makegen(1, 18, 1000, 0,1, 1,1)
/* Sine waveform */
makegen(2, 10, 1000, 1)

dur = 5.0

/* Long TRANS note — reads from aux 0 for full duration, unity pitch */
TRANS(0, 0, dur, 1, 0.0)

/* Writer 1: t=0 to t=1 */
WAVETABLE(0, 1, 10000, 440)

/* Gap: t=1 to t=2 — bus produces silence */

/* Writer 2: t=2 to t=3 */
WAVETABLE(2, 1, 10000, 880)

/* Gap: t=3 to t=4 */

/* Writer 3: t=4 to t=5 */
WAVETABLE(4, 1, 10000, 660)

print("instbus_trans_reuse_test: TRANS(5s) with 3 staggered WAVETABLEs on aux 0")
print("Expected: peak ~10000 (from WAVETABLE segments)")
