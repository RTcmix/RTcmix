rtsetparams(44100, 2)
load("WIGGLE")

dur = 40.0
gaindb = 85.0
pitch = 9.01
mfreq = cpspch(pitch + 10.00)

makegen(1, 4, 2000, 0,0,1, dur*.08,1,0, dur*.7,1,-3, dur,0)

makegen(2, 10, 5000, 1, .4, .3, .2, .1, .1, .05, .01) /* car wave */
makegen(3, 18, 10, 0,0,1,0)                           /* car gliss */
makegen(4, 10, 2000, 1)                               /* mod wave */
makegen(5, 18, 20, 0,mfreq, 1,mfreq)                  /* mod freq */
makegen(6, 18, 2000, 0,0, 1,7)                        /* mod depth */
makegen(7, 18, 2000, 0,3000, 1,8000, 3,900)           /* filt cf */
makegen(8, 18, 2000, 0,.1, 1,.5)                      /* pan */

depth_type = 2
filt_type = 1
filt_steep = 12
amp = ampdb(gaindb)

WIGGLE(start=0, dur, amp, pitch, depth_type, filt_type, filt_steep)

makegen(3, 18, 1000, 0,0,1,0.001)                     /* car gliss */
makegen(8, 18, 2000, 0,.6, 1,1)                       /* pan */
WIGGLE(start+.01, dur, amp, pitch, depth_type, filt_type, filt_steep)

/* bass later on */
start = start + (dur * .4)
dur = (dur - start) + 2
makegen(1, 4, 2000, 0,0,1, dur*.5,1,0, dur*.7,1,-3, dur,0)
gaindb = 75.0
pitch = 5.10
makegen(2, 10, 2000, 1,.5,.3,.1)                      /* car wave */
makegen(3, 18, 10, 0,0,1,0)                           /* cancel car gliss */
makegen(8, 18, 20, 0,0, 1,0)                          /* pan */
WIGGLE(start, dur, ampdb(gaindb), pitch)
makegen(8, 18, 20, 0,1, 1,1)                          /* pan */
WIGGLE(start, dur, ampdb(gaindb), pitch+.001)
