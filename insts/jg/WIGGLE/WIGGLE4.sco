rtsetparams(44100, 2)
load("WIGGLE")

dur = 25
amp = 3000
pitch = 10.00

makegen(1, 4, 2000, 0,0,2, dur*.1,1,0, dur*.4,1,-3, dur,0)

makegen(2, 10, 8000, 1)                /* car wave */
makegen(3, 20, 300, 2, 0, -1.00,2.00)  /* random gliss */

makegen(4, 10, 1000, 1)                /* mod waveform */
makegen(5, 18, 20, 0,200, 1,200)       /* mod pitch */
makegen(6, 18, 2000, 0,20, 1,20)       /* mod depth */

makegen(-7, 4, 2000, 0,1000,-4, 1,1)   /* filter cf */
makegen(8, 18, 2000, 0,.2, 1,.5, 2,1)  /* pan */

filt_type = 2                          /* highpass */
filt_steep = 20

WIGGLE(st=0, dur, amp, pitch, 2, filt_type, filt_steep)

makegen(8, 18, 2000, 0,1, 1,0)         /* pan */
WIGGLE(st=0.1, dur, amp, pitch+.01, 2, filt_type, filt_steep)
