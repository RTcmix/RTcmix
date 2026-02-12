/* instbus_stereo_merge_test.sco
 *
 * Test: Two independent mono instruments merge into stereo output
 *
 * Routing:
 *   WAVETABLE(440Hz) -> aux 0 -> MIX -> out 0 (left)
 *   WAVETABLE(550Hz) -> aux 3 -> MIX -> out 1 (right)
 *
 * Uses non-sequential aux buses (0 and 3) to verify bus indexing.
 * Expected: Left channel = 440Hz (A4), Right channel = 550Hz (C#5)
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")
load("MIX")

/* Envelope */
makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
/* Waveform */
makegen(2, 10, 1000, 1)

dur = 2.0
amp = 8000

/* 440Hz on aux 0 -> left channel */
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, amp, 440)

bus_config("MIX", "aux 0 in", "out 0")
MIX(0, 0, dur, 1)

/* 550Hz on aux 3 -> right channel */
bus_config("WAVETABLE", "aux 3 out")
WAVETABLE(0, dur, amp, 550)

bus_config("MIX", "aux 3 in", "out 1")
MIX(0, 0, dur, 1)

print("instbus_stereo_merge_test: Two mono sources (aux 0, aux 3) -> stereo output")
print("Expected: Left=440Hz, Right=550Hz")