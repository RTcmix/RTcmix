/* instbus_complex_quad_test.sco
 *
 * Test: Complex multi-stage routing to quad output
 *
 * Routing:
 *   WAVETABLE(110Hz) -> aux 0 -> TRANS(+12) -> aux 1 -\
 *                                                      >-> MIX -> aux 10 -> out 0 (FL)
 *   WAVETABLE(110Hz) -> aux 2 -> TRANS(+19) -> aux 3 -/
 *
 *   WAVETABLE(110Hz) -> aux 4 -> TRANS(+24) -> aux 5 -\
 *                                                      >-> MIX -> aux 11 -> out 1 (FR)
 *   WAVETABLE(110Hz) -> aux 6 -> TRANS(+28) -> aux 7 -/
 *
 *   WAVETABLE(55Hz)  -> aux 8 -----------------------> MIX -> aux 12 -> out 2 (RL)
 *
 *   WAVETABLE(55Hz)  -> aux 9 -> TRANS(+24) --------> MIX -> aux 13 -> out 3 (RR)
 *
 * Front speakers: Harmonic combinations of 110Hz fundamental
 *   FL: 220Hz (octave) + 330Hz (5th above octave) mixed
 *   FR: 440Hz (2 octaves) + 554Hz (major 3rd above 2 octaves) mixed
 * Rear speakers: Sub-bass
 *   RL: 55Hz (sub-bass fundamental)
 *   RR: 220Hz (transposed sub-bass)
 */

rtsetparams(44100, 4, 512)
load("WAVETABLE")
load("TRANS")
load("MIX")

/* Envelope */
makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
/* Waveform - add some harmonics for richer sound */
makegen(2, 10, 1000, 1, 0.5, 0.3, 0.2)

dur = 3.0
transdur = 6.0
amp = 5000

/* ===== FRONT LEFT: 220Hz + 330Hz ===== */
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, amp, 110)
bus_config("TRANS", "aux 0 in", "aux 1 out")
TRANS(0, 0, transdur, 1, 1.0)  /* +12 semitones: 110 -> 220Hz */

bus_config("WAVETABLE", "aux 2 out")
WAVETABLE(0, dur, amp, 110)
bus_config("TRANS", "aux 2 in", "aux 3 out")
TRANS(0, 0, transdur, 1, 1.07)  /* +19 semitones: 110 -> 330Hz */

bus_config("MIX", "aux 1 in", "aux 10 out")
MIX(0, 0, dur, 0.7)
bus_config("MIX", "aux 3 in", "aux 10 out")
MIX(0, 0, dur, 0.7)

bus_config("MIX", "aux 10 in", "out 0")
MIX(0, 0, dur, 1)

/* ===== FRONT RIGHT: 440Hz + 554Hz ===== */
bus_config("WAVETABLE", "aux 4 out")
WAVETABLE(0, dur, amp, 110)
bus_config("TRANS", "aux 4 in", "aux 5 out")
TRANS(0, 0, transdur, 1, 2.0)  /* +24 semitones: 110 -> 440Hz */

bus_config("WAVETABLE", "aux 6 out")
WAVETABLE(0, dur, amp, 110)
bus_config("TRANS", "aux 6 in", "aux 7 out")
TRANS(0, 0, transdur, 1, 2.04)  /* +28 semitones: 110 -> ~554Hz (major 3rd above 440) */

bus_config("MIX", "aux 5 in", "aux 11 out")
MIX(0, 0, dur, 0.7)
bus_config("MIX", "aux 7 in", "aux 11 out")
MIX(0, 0, dur, 0.7)

bus_config("MIX", "aux 11 in", "out 1")
MIX(0, 0, dur, 1)

/* ===== REAR LEFT: 55Hz sub-bass ===== */
bus_config("WAVETABLE", "aux 8 out")
WAVETABLE(0, dur, amp * 1.5, 55)  /* boost sub-bass */

bus_config("MIX", "aux 8 in", "out 2")
MIX(0, 0, dur, 1)

/* ===== REAR RIGHT: 220Hz (transposed sub) ===== */
bus_config("WAVETABLE", "aux 9 out")
WAVETABLE(0, dur, amp, 55)
bus_config("TRANS", "aux 9 in", "aux 13 out")
TRANS(0, 0, transdur, 1, 2.0)  /* +24 semitones: 55 -> 220Hz */

bus_config("MIX", "aux 13 in", "out 3")
MIX(0, 0, dur, 1)

print("instbus_complex_quad_test: Multi-stage processing to quad output")
print("FL: 220+330Hz, FR: 440+554Hz, RL: 55Hz, RR: 220Hz")