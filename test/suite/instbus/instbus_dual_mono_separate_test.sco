/* instbus_dual_mono_separate_test.sco
 *
 * Test: Two mono instruments routed to separate stereo outputs
 *
 * Routing:
 *   WAVETABLE(440Hz) -> aux 3 -> MIX -> out 0 (left only)
 *   WAVETABLE(880Hz) -> aux 8 -> MIX -> out 1 (right only)
 *
 * Each aux bus goes to a separate output channel.
 * Expected: 440Hz in left ear ONLY, 880Hz in right ear ONLY
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

/* 440Hz on aux 3 -> left channel */
bus_config("WAVETABLE", "aux 3 out")
WAVETABLE(0, dur, amp, 440)

bus_config("MIX", "aux 3 in", "out 0")
MIX(0, 0, dur, 1)

/* 659Hz (perfect 5th above 440) on aux 8 -> right channel */
bus_config("WAVETABLE", "aux 8 out")
WAVETABLE(0, dur, amp, 659)

bus_config("MIX", "aux 8 in", "out 1")
MIX(0, 0, dur, 1)

print("instbus_dual_mono_separate_test: aux 3 -> left, aux 8 -> right")
print("Expected: Left=440Hz ONLY, Right=659Hz (perfect 5th) ONLY")