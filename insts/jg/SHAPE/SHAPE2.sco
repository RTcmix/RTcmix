rtsetparams(44100, 2)
load("WAVETABLE")
load("SHAPE")

bus_config("WAVETABLE", "aux 0 out")
bus_config("SHAPE", "aux 0 in", "out 0-1")

dur = 20
freq = 60

amp = 20000
makegen(2, 10, 2000, 1, .3, .1)
WAVETABLE(start=0, dur, amp, freq)

/* square wave transfer function */
makegen(2, 10, 1000, 1, 0, 1/3, 0, 1/5, 0, 1/7, 0, 1/9, 0, 1/11, 0, 1/13)

/* sample-and-hold distortion index */
speed = 7                              /* sh changes per second */
shsize = dur * speed
makegen(3, 20, shsize, 0, seed=1)
copygen(3, 3, 4000, 0)                 /* avoid interpolation btw. rand vals */
minidx = 0.0
maxidx = 3.0

makegen(99, 4, 1000, 0,1,-1, 1,.3)  /* normalization function */

amp = 0.8
setline(0,1, dur-.03,1, dur,0)
reset(20000)

SHAPE(start, inskip=0, dur, amp, minidx, maxidx, 99, 0, 1)

/* vary distortion index for other channel */
makegen(3, 20, shsize, 0, seed + 1)
copygen(3, 3, 4000, 0)
SHAPE(start, inskip=0, dur, amp, minidx, maxidx, 99, 0, 0)

