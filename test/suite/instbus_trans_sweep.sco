/* instbus_trans_sweep.sco
 *
 * Test: 2-TRANS chain with configurable transposition
 * Usage: set TRANSP before running to change the shift value
 *
 * Routing:
 *   WAVETABLE(440Hz) -> aux 0 -> TRANS(TRANSP) -> aux 1 -> TRANS(TRANSP) -> aux 2 -> STEREO -> out 0-1
 */

rtsetparams(44100, 2, 512)
load("WAVETABLE")
load("TRANS")
load("STEREO")

dur = 0.5

/* Source */
bus_config("WAVETABLE", "aux 0 out")

/* First TRANS */
bus_config("TRANS", "aux 0 in", "aux 1 out")

/* Waveform - sine */
makegen(2, 10, 1000, 1)

/* 440Hz source, constant amplitude, no envelope */
WAVETABLE(0, dur, 10000, 440)

/* First shift */
TRANS(0, 0, dur, 1, TRANSP)

/* Second TRANS */
bus_config("TRANS", "aux 1 in", "aux 2 out")
TRANS(0, 0, dur, 1, TRANSP)

/* Final output */
bus_config("STEREO", "aux 2 in", "out 0-1")
STEREO(0, 0, dur, 1, 0.5)
