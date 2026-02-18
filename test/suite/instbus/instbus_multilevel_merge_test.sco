/* instbus_multilevel_merge_test.sco
 *
 * Test: Three-level aux bus cascade with merging at each level
 *
 * Routing:
 *   Level 0 (sources):
 *     WAVETABLE(440Hz, amp 4000) -> aux 0
 *     WAVETABLE(440Hz, amp 4000) -> aux 3
 *
 *   Level 1 (merge + new source):
 *     MIX(aux 0 + aux 3) -> aux 5          (carries 8000 peak)
 *     WAVETABLE(440Hz, amp 4000) -> aux 7
 *
 *   Level 2 (merge):
 *     MIX(aux 5 + aux 7) -> aux 10         (carries 12000 peak)
 *
 *   Output:
 *     MIX(aux 10) -> out 0-1
 *
 * All sources are 440Hz, in phase. Peak is the additive sum.
 * Tests: Multi-level cascade, merging at different depths, non-sequential bus IDs
 *
 * Expected peak: ~12000 (3 sources x 4000)
 *   If level 0 merge fails:  ~8000 (one source + aux 7)
 *   If level 2 merge fails:  ~8000 or ~4000
 *   If entire chain breaks:  0
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")

makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
makegen(2, 10, 1000, 1)

dur = 2.0
amp = 4000

/* Level 0: Two sources on separate aux buses */
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, amp, 440)

bus_config("WAVETABLE", "aux 3 out")
WAVETABLE(0, dur, amp, 440)

/* Level 1: Merge aux 0 + aux 3 into aux 5 */
bus_config("MIX", "aux 0 in", "aux 3 in", "aux 5 out")
MIX(0, 0, dur, 1, 0, 0)

/* Level 1: Additional source on aux 7 */
bus_config("WAVETABLE", "aux 7 out")
WAVETABLE(0, dur, amp, 440)

/* Level 2: Merge aux 5 + aux 7 into aux 10 */
bus_config("MIX", "aux 5 in", "aux 7 in", "aux 10 out")
MIX(0, 0, dur, 1, 0, 0)

/* Output: aux 10 to stereo */
bus_config("MIX", "aux 10 in", "out 0-1")
MIX(0, 0, dur, 1, 0, 1)

print("instbus_multilevel_merge_test: 3-level cascade with merges")
print("Expected peak: ~12000 (3 x 4000)")