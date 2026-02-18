/* instbus_dual_mono_to_stereo_test.sco
 *
 * Test: Two independent mono instruments become stereo input
 *
 * Routing:
 *   WAVETABLE(440Hz) -> aux 3 --\
 *                                >-- STEREO -> out 0-1
 *   WAVETABLE(880Hz) -> aux 8 --/
 *
 * Key test: Non-sequential buses (3 and 8) specified as separate arguments.
 *
 * Expected: 440Hz and 880Hz (octave apart) mixed to stereo output
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")
load("STEREO")

/* Envelope */
makegen(1, 24, 1000, 0,0, 0.1,1, 0.9,1, 1,0)
/* Waveform */
makegen(2, 10, 1000, 1)

dur = 2.0
amp = 8000

/* Left channel source: 440Hz on aux 3 */
bus_config("WAVETABLE", "aux 3 out")
WAVETABLE(0, dur, amp, 440)

/* Right channel source: 659Hz (perfect 5th above 440) on aux 8 */
bus_config("WAVETABLE", "aux 8 out")
WAVETABLE(0, dur, amp, 659)

/* STEREO reads from aux 3 and aux 8 as separate arguments */
bus_config("STEREO", "aux 3 in", "aux 8 in", "out 0-1")

/* Pan per input channel: ch0(aux3)=1.0 (left), ch1(aux8)=0.0 (right) */
STEREO(0, 0, dur, 1, 1.0, 0.0)

print("instbus_dual_mono_to_stereo_test: Two mono sources (aux 3, aux 8) as stereo input")
print("Expected: 440Hz left, 659Hz (perfect 5th) right")