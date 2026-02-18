/* instbus_diamond_test.sco
 *
 * Test: Diamond topology - single source splits, processes differently, then merges
 *
 * Routing:
 *                        /-> TRANS(+7semi) -> aux 2 -\
 *   WAVETABLE(220Hz) -> aux 0                         >-> MIX -> aux 5 -> STEREO -> out 0-1
 *                        \-> TRANS(+12semi) -> aux 3 -/
 *
 * Single source feeds two TRANS instances (different intervals),
 * results mix together, then output to stereo.
 *
 * Expected: Mixed 330Hz (perfect 5th) and 440Hz (octave) = power chord
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")
load("TRANS")
load("MIX")
load("STEREO")

/* Envelope */
makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
/* Waveform */
makegen(2, 10, 1000, 1)

dur = 2.0
amp = 6000

/* Source: 220Hz on aux 0 */
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, amp, 220)

/* Branch 1: +7 semitones (perfect 5th) -> 330Hz */
bus_config("TRANS", "aux 0 in", "aux 2 out")
TRANS(0, 0, dur, 1, 0.07)  /* +7 semitones in oct.pc */

/* Branch 2: +12 semitones (octave) -> 440Hz */
bus_config("TRANS", "aux 0 in", "aux 3 out")
TRANS(0, 0, dur, 1, 1.0)  /* +12 semitones in oct.pc */

/* Merge: combine aux 2 and 3 into aux 5 */
bus_config("MIX", "aux 2-3 in", "aux 5 out")
MIX(0, 0, dur, 0.7, 0, 0)  /* both inputs to single mono output */

/* Output to stereo */
bus_config("STEREO", "aux 5 in", "out 0-1")
STEREO(0, 0, dur, 1, 0.5)

print("instbus_diamond_test: Source splits to two TRANS, merges via MIX, outputs stereo")
print("Expected: Mixed 330Hz + 440Hz (power chord sound)")