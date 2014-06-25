rtsetparams(44100, 2)
load("WAVETABLE")
load("PAN")

/* feed wavetable into panner */
bus_config("WAVETABLE", "aux 0 out")
bus_config("PAN", "aux 0 in", "out 0-1")

dur = 10.0
amp = 6000
pitch = 8.00
makegen(2, 10, 5000, 1, .4, .3, .2, .1)
setline(0,0, 1,1, 7,1, 10,0)
WAVETABLE(0, dur, amp, pitch)

speed = 10        /* scans through gen table per second */
makegen(
   -2, 9, 10000,  /* negative slot number prevents rescaling to [-1,1] */
   0, .5, 90,     /* this DC component lifts the other one above 0... */
   speed*dur, .5, 0)

reset(22051)      /* lower control rates can produce zipper noise */
PAN(0, 0, dur, amp=1, 0, 1)   /* disable constant-power panning */

