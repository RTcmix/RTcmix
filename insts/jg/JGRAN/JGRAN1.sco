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


JGRAN(start=0, dur, amp, seed=1, type=1, ranphase=1)

