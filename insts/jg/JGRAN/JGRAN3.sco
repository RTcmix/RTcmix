rtsetparams(44100, 2);
load("JGRAN")
load("REVERBIT")
bus_config("JGRAN", "aux 0-1 out")
bus_config("REVERBIT", "aux 0-1 in",  "out 0-1")

masteramp = 1.0

dur = 16

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
makegen(10, 18, 1000, 0,65, 1,65)           /* min */
makegen(11, 18, 1000, 0,80, 1,80)           /* max */

/* grain density */
makegen(12, 18, 1000, 0,1, 1,1, 2,.8)       /* slightly decreasing density */

/* grain stereo location */
makegen(13, 18, 1000, 0,.5, 1,.5)           /* image centered in middle */

/* grain stereo location randomization */
makegen(14, 18, 1000, 0,0, 1,1)             /* increasingly randomized */


JGRAN(start=0, dur, amp=.5, seed=1, type=1, ranphase=1)

makegen(3, 10, 10000, 1, .5, .3, .2, .1)
makegen(6, 18, 1000, 0,900, 1,840)
makegen(7, 18, 1000, 0,900, 1,1000)
makegen(14, 18, 1000, 0,1, 1,0)
JGRAN(start=0, dur, amp=1, seed=2, type=0, ranphase=1)

/* --------------------------------------------------------------- reverb --- */
amp = 1.0 * masteramp
revtime = 1.0
revpct = .8
rtchandel = .03
cf = 2000

setline(0,1, 1,1)

REVERBIT(st=0, insk=0, dur, amp, revtime, revpct, rtchandel, cf)

