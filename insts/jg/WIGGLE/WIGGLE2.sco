rtsetparams(44100, 1)
load("WIGGLE")

dur = 10
two_layers = 1  /* 0: one layer, 1: two layers */
amp = 800
pitch = 11.00

setline(0,1, dur-.1,1, dur,0)

makegen(2, 10, 8000, 1,.3,.1)       /* car waveform */

/* -------------------------------------------------------------------------- */
/* Use gliss function, created with gen 20, to provide random "vibrato."
   This shows how to compute gen size to get the vibrato rate you want.
   Note that WIGGLE doesn't interpolate between values of the gliss
   function, so you get a clear stair-step effect, like an old analog
   sequencer.  If you want smooth transitions between the steps, use
   the mod oscillator instead.  Then you could also have varying speed,
   which can do with the gliss function.
*/
vib_speed = 10 /* in Hz */
dist_type = 0  /* 0=even, 1=low, 2=high, 3=triangle, 4=gaussian, 5=cauchy */
seed = 1

/* down as much as a perfect fifth, up as much as an octave */
min = -0.07    /* in oct.pc */
max = 1.00

gen_size = vib_speed * dur

/* gliss function values are in linear octaves, thus the octpch conversions. */
makegen(3, 20, gen_size, dist_type, seed, octpch(min), octpch(max))

WIGGLE(st=0, dur, amp, pitch)

/* -------------------------------------------------------------------------- */
if (two_layers) {
   /* same as above, but different seed */
   makegen(3, 20, gen_size, dist_type, seed+1, octpch(min), octpch(max))
   pitch = pitch - 3  /* 3 octaves below */
   WIGGLE(st=0, dur, amp, pitch)
}
