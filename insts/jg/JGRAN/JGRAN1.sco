rtsetparams(44100, 2);
load("JGRAN")
bus_config("JGRAN", "out0-1")

dur = 16
amp = 1

/* overall amplitude envelope */
setline(0,0, 1,1, 2,1, 4,0)

/* grain envelope */
makegen(2, 25, 10000, 1)                    /* hanning window */

/* grain waveform */
makegen(3, 10, 10000, 1)                    /* sine wave */

/* modulation frequency multiplier */
makegen(4, 18, 1000, 0,2, 1,2.1)            /* slightly increasing multiplier */

/* index of modulation envelope (per grain) */
makegen(5, 18, 1000, 0,0, 1,5)              /* increasing index */

/* grain frequency */
makegen(6, 18, 1000, 0,500, 1,500)          /* constant minimum */
makegen(7, 18, 1000, 0,500, 1,550)          /* increasing maximum */

/* grain speed */
makegen(8, 18, 1000, 0,100, 1,10)           /* decreasing minimum */
makegen(9, 18, 1000, 0,100, 1,100)          /* constant maximum */

/* grain intensity (decibels above 0) */
makegen(10, 18, 1000, 0,80, 1,80)           /* min */
makegen(11, 18, 1000, 0,80, 1,80)           /* max */

/* grain density */
makegen(12, 18, 1000, 0,1, 1,1, 2,.8)       /* slightly decreasing density */

/* grain stereo location */
makegen(13, 18, 1000, 0,.5, 1,.5)           /* image centered in middle */

/* grain stereo location randomization */
makegen(14, 18, 1000, 0,0, 1,1)             /* increasingly randomized */


JGRAN(start=0, dur, amp, seed=.1, type=1, ranphase=1)


/* JGRAN - a granular synthesis with FM or AS grains

   This was derived from a Cecilia module (StochasticGrains) by
   Mathieu Bezkorowajny and Jean Piche. See also Mara Helmuth's
   sgran (RTcmix/insts.std) for more control over randomness.

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = random seed (if 0, seed from system clock) [default: 0]
   p4 = oscillator configuration (0: additive, 1: FM, 2: sampled) [default: 0]
   p5 = randomize oscillator starting phase (0: no, 1: yes) [default: yes]

   Function table (makegen) assignments:

    1   overall amplitude envelope (or use setline)
    2   grain envelope
    3   grain waveform
    4   modulator frequency multiplier            (can skip if p4 is not 1)
    5   index of modulation envelope (per grain)  (can skip if p4 is not 1)
    6   minimum grain frequency
    7   maximum grain frequency
    8   minumum grain speed
    9   maximum grain speed
   10   minumum grain intensity
   11   maximum grain intensity
   12   grain density
   13   grain stereo location                     (can skip if output is mono)
   14   grain stereo location randomization       (can skip if output is mono)

   NOTES:
     1. Produces only one stream of non-overlapping grains. To get more
        streams, call JGRAN more than once (maybe with different seeds).
     2. Uses non-interpolating oscillators for efficiency, so make large
        tables for the grain waveform and grain envelope.
     3. For functions 5-11, either use gen18 or make the slot number negative,
        to tell the gen routine not to rescale values to fit between 0 and 1.
     4. Grains within one stream (note) never overlap. (Use sgran for this.)
     5. Ability to use a sampled waveform is not implemented yet.

   John Gibson (johngibson@virginia.edu), 4/15/00.
*/

