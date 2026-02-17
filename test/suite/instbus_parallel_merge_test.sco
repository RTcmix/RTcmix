/* instbus_parallel_merge_test.sco
 *
 * Test: Parallel processing chains that merge into stereo
 *
 * Routing:
 *   WAVETABLE(220Hz) -> aux 0 -> TRANS(+12semi) -> aux 10 --\
 *                                                            >-- STEREO -> out 0-1
 *   WAVETABLE(220Hz) -> aux 5 -> TRANS(-12semi) -> aux 11 --/
 *
 * Two parallel chains: one shifts up an octave, one shifts down.
 * Expected: Left = 440Hz, Right = 110Hz
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")
load("TRANS")
load("STEREO")

/* Envelope */
makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
/* Waveform */
makegen(2, 10, 1000, 1)

dur = 2.0
amp = 8000

/* === Upper chain: 220Hz -> +1 octave -> 440Hz === */
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, amp, 220)

bus_config("TRANS", "aux 0 in", "aux 10 out")
TRANS(0, 0, dur, 1, 1.0)  /* +12 semitones (1 octave) in oct.pc */

/* === Lower chain: 220Hz -> -1 octave -> 110Hz === */
bus_config("WAVETABLE", "aux 5 out")
WAVETABLE(0, dur, amp, 220)

bus_config("TRANS", "aux 5 in", "aux 11 out")
TRANS(0, 0, dur, 1, -1.0)  /* -12 semitones (1 octave down) in oct.pc */

/* === Merge: aux 10 -> left, aux 11 -> right === */
bus_config("STEREO", "aux 10 in", "out 0-1")
STEREO(0, 0, dur, 1, 0.0)  /* hard left */

bus_config("STEREO", "aux 11 in", "out 0-1")
STEREO(0, 0, dur, 1, 1.0)  /* hard right */

print("instbus_parallel_merge_test: Two parallel TRANS chains merge to stereo")
print("Expected: Left=440Hz (octave up), Right=110Hz (octave down)")