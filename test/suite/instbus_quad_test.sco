/* instbus_quad_test.sco
 *
 * Test: Four independent mono sources to quad output
 *
 * Routing:
 *   WAVETABLE(262Hz, C4)  -> aux 0 --\
 *   WAVETABLE(330Hz, E4)  -> aux 2 ---\
 *                                      >-- MIX -> out 0-3
 *   WAVETABLE(392Hz, G4)  -> aux 5 ---/
 *   WAVETABLE(523Hz, C5)  -> aux 7 --/
 *
 * Uses non-sequential aux buses to verify indexing.
 * Expected: C major chord with each note in a different speaker
 *   Front Left (0):  C4 (262Hz)
 *   Front Right (1): E4 (330Hz)
 *   Rear Left (2):   G4 (392Hz)
 *   Rear Right (3):  C5 (523Hz)
 */

rtsetparams(44100, 4, 512)
load("WAVETABLE")
load("MIX")

/* Envelope */
makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
/* Waveform */
makegen(2, 10, 1000, 1)

dur = 2.0
amp = 6000

/* C4 on aux 0 -> output 0 (front left) */
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, amp, 262)

/* E4 on aux 2 -> output 1 (front right) */
bus_config("WAVETABLE", "aux 2 out")
WAVETABLE(0, dur, amp, 330)

/* G4 on aux 5 -> output 2 (rear left) */
bus_config("WAVETABLE", "aux 5 out")
WAVETABLE(0, dur, amp, 392)

/* C5 on aux 7 -> output 3 (rear right) */
bus_config("WAVETABLE", "aux 7 out")
WAVETABLE(0, dur, amp, 523)

/* MIX routes each aux to corresponding output channel */
bus_config("MIX", "aux 0 in", "out 0")
MIX(0, 0, dur, 1)

bus_config("MIX", "aux 2 in", "out 1")
MIX(0, 0, dur, 1)

bus_config("MIX", "aux 5 in", "out 2")
MIX(0, 0, dur, 1)

bus_config("MIX", "aux 7 in", "out 3")
MIX(0, 0, dur, 1)

print("instbus_quad_test: Four mono sources (aux 0,2,5,7) -> quad output")
print("Expected: C major chord - C4(FL), E4(FR), G4(RL), C5(RR)")