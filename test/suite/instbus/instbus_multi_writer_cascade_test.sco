/* instbus_multi_writer_cascade_test.sco
 *
 * Test: Multiple writers at each level of a two-stage cascade
 *
 * Routing:
 *   Stage 0:
 *     WAVETABLE(440Hz, 3000) -> aux 0     (2 writers -> aux 0 peak 6000)
 *     WAVETABLE(440Hz, 3000) -> aux 0
 *
 *     WAVETABLE(440Hz, 3000) -> aux 1     (2 writers -> aux 1 peak 6000)
 *     WAVETABLE(440Hz, 3000) -> aux 1
 *
 *   Stage 1 (two MIX writers to same bus):
 *     MIX(aux 0) -> aux 4                 (carries 6000)
 *     MIX(aux 1) -> aux 4                 (carries 6000, sums to 12000)
 *
 *   Output:
 *     MIX(aux 4) -> out 0-1
 *
 * Tests: Multiple writers at every level, fan-in at intermediate bus
 *
 * Expected peak: ~12000 (4 x 3000)
 *   If one source bus fails: ~6000
 *   If intermediate fan-in fails: ~6000
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")

makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
makegen(2, 10, 1000, 1)

dur = 2.0
amp = 3000

/* Two writers to aux 0 */
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, amp, 440)
WAVETABLE(0, dur, amp, 440)

/* Two writers to aux 1 */
bus_config("WAVETABLE", "aux 1 out")
WAVETABLE(0, dur, amp, 440)
WAVETABLE(0, dur, amp, 440)

/* Both feed into aux 4 (two MIX writers to same bus) */
bus_config("MIX", "aux 0 in", "aux 4 out")
MIX(0, 0, dur, 1)

bus_config("MIX", "aux 1 in", "aux 4 out")
MIX(0, 0, dur, 1)

/* Output */
bus_config("MIX", "aux 4 in", "out 0-1")
MIX(0, 0, dur, 1, 0, 1)

print("instbus_multi_writer_cascade_test: 2 writers per bus, 2 stages")
print("Expected peak: ~12000 (4 x 3000)")