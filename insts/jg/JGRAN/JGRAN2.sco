rtsetparams(44100, 2);
load("JGRAN")
bus_config("JGRAN", "out0-1")

dur = 10
amp = .5

/* overall amplitude envelope */
setline(0,0, 6,1, 9,1, 10,0)

/* grain envelope */
makegen(2, 18, 10000, 0,0, 1,1, 10,.2, 20,0)
makegen(2, 25, 10000, 1)

/* grain waveform */
makegen(3, 10, 10000, 1, .1, .05)

/* modulation frequency multiplier */
makegen(4, 18, 1000, 0,1.3, 1,1.7)
makegen(4, 20, 1000, 2)

/* index of modulation envelope (per grain) */
makegen(5, 18, 1000, 0,0, 1,6)

/* grain frequency */
makegen(6, 18, 1000, 0,500, 1,500)          /* min */
makegen(7, 18, 1000, 0,500, 1,500)          /* max */

/* grain speed */
makegen(8, 18, 1000, 0,20, 1,20)            /* min */
makegen(9, 18, 1000, 0,90, 1,90)            /* max */

/* grain intensity (decibels above 0) */
makegen(10, 18, 1000, 0,80, 1,80)           /* min */
makegen(11, 18, 1000, 0,80, 1,80)           /* max */

/* grain density */
makegen(12, 18, 1000, 0,1, 1,1)

/* grain stereo location */
makegen(13, 18, 1000, 0,.5, 1,.5)

/* grain stereo location randomization */
makegen(14, 18, 1000, 0,0, 1,1)


JGRAN(start=0, dur, amp, seed=1, type=1)

/* a second grain stream, with some different params */
setline(0,0, 1,1, 4,1, 10,0)
makegen(6, 18, 1000, 0,1000, 1,1000)        /* min */
makegen(7, 18, 1000, 0,1100, 1,1100)        /* max */
makegen(14, 18, 1000, 0,1, 1,0)
amp = 2
JGRAN(start=0, dur, amp, seed=2, type=0)


