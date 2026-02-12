/* instbus_deep_chain_test.sco
 *
 * Test: Deep 5-stage aux bus chain, single signal
 *
 * Routing:
 *   WAVETABLE(440Hz, 20000) -> aux 0
 *     -> MIX(0.9) -> aux 2
 *       -> MIX(0.9) -> aux 5
 *         -> MIX(0.9) -> aux 8
 *           -> MIX(0.9) -> aux 11
 *             -> MIX(0.9) -> out 0-1
 *
 * Tests: Deep chain of 5 AUX_TO_AUX stages, signal integrity through many hops
 *
 * Expected peak: 20000 * 0.9^5 = ~11809
 *   If any stage breaks: 0 (chain is severed)
 *   If gain is wrong: different amplitude
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")

makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
makegen(2, 10, 1000, 1)

dur = 2.0

/* Source */
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, 20000, 440)

/* Stage 1 */
bus_config("MIX", "aux 0 in", "aux 2 out")
MIX(0, 0, dur, 0.9)

/* Stage 2 */
bus_config("MIX", "aux 2 in", "aux 5 out")
MIX(0, 0, dur, 0.9)

/* Stage 3 */
bus_config("MIX", "aux 5 in", "aux 8 out")
MIX(0, 0, dur, 0.9)

/* Stage 4 */
bus_config("MIX", "aux 8 in", "aux 11 out")
MIX(0, 0, dur, 0.9)

/* Stage 5: to output */
bus_config("MIX", "aux 11 in", "out 0-1")
MIX(0, 0, dur, 0.9, 0, 1)

print("instbus_deep_chain_test: 5-stage aux chain (0->2->5->8->11->out)")
print("Expected peak: ~11809 (20000 * 0.9^5)")