/* instbus_chain_test.sco
 *
 * Test: Multi-stage processing chain with pitch shifting
 *
 * Routing:
 *   WAVETABLE(220Hz) -> aux 0 -> TRANS(+7semi) -> aux 1 -> TRANS(+5semi) -> aux 2 -> STEREO -> out 0-1
 *
 * Chain: 220Hz -> 330Hz (+7 semitones) -> 440Hz (+5 semitones) -> stereo output
 * Expected: 440Hz tone (A4) in stereo
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")
load("TRANS")
load("STEREO")

/* Source */
bus_config("WAVETABLE", "aux 0 out")

/* First pitch shift: +7 semitones (perfect 5th) */
bus_config("TRANS", "aux 0 in", "aux 1 out")

/* Envelope */
makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
/* Waveform */
makegen(2, 10, 1000, 1)

/* 220Hz source */
WAVETABLE(0, 2, 10000, 220)

/* +7 semitones in octave.pc format (0.07 = 0 octaves, 7 semitones) */
TRANS(0, 0, 2, 1, 0.07)

/* Second pitch shift: reconfigure TRANS for next stage */
bus_config("TRANS", "aux 1 in", "aux 2 out")

/* +5 semitones in octave.pc format (0.05 = 0 octaves, 5 semitones) */
/* Combined: 220 * 2^(7/12) * 2^(5/12) = 220 * 2 = 440Hz */
TRANS(0, 0, 2, 1, 0.05)

/* Final output */
bus_config("STEREO", "aux 2 in", "out 0-1")
STEREO(0, 0, 2, 1, 0.5)

print("instbus_chain_test: WAVETABLE(220) -> TRANS(+5th) -> TRANS(+4th) -> STEREO")
print("Expected: 440Hz tone (A4)")