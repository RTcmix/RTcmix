/* instbus_asymmetric_tree_test.sco
 *
 * Test: Asymmetric tree - branches of different depth converge on one bus
 *
 * Routing:
 *   Deep branch (3 hops):
 *     WAVETABLE(440Hz, 4000) -> aux 0 -> MIX -> aux 2 -> MIX -> aux 6
 *
 *   Medium branch (2 hops):
 *     WAVETABLE(440Hz, 4000) -> aux 3 -> MIX -> aux 6
 *
 *   Shallow branch (1 hop):
 *     WAVETABLE(440Hz, 4000) -> aux 6
 *
 *   Three writers converge on aux 6:
 *     MIX(aux 6) -> out 0-1
 *
 * Tests: Different-depth branches merging, 3 writers to same bus from
 *        different routing depths
 *
 * Expected peak: ~12000 (3 x 4000)
 *   If deep branch fails:    ~8000
 *   If medium branch fails:  ~8000
 *   If shallow branch fails: ~8000
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")

makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
makegen(2, 10, 1000, 1)

dur = 2.0
amp = 4000

/* Deep branch: aux 0 -> aux 2 -> aux 6 */
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, amp, 440)

bus_config("MIX", "aux 0 in", "aux 2 out")
MIX(0, 0, dur, 1)

bus_config("MIX", "aux 2 in", "aux 6 out")
MIX(0, 0, dur, 1)

/* Medium branch: aux 3 -> aux 6 */
bus_config("WAVETABLE", "aux 3 out")
WAVETABLE(0, dur, amp, 440)

bus_config("MIX", "aux 3 in", "aux 6 out")
MIX(0, 0, dur, 1)

/* Shallow branch: direct to aux 6 */
bus_config("WAVETABLE", "aux 6 out")
WAVETABLE(0, dur, amp, 440)

/* Output */
bus_config("MIX", "aux 6 in", "out 0-1")
MIX(0, 0, dur, 1, 0, 1)

print("instbus_asymmetric_tree_test: 3 branches depth 3/2/1 -> aux 6 -> out")
print("Expected peak: ~12000 (3 x 4000)")