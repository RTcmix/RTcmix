/* This script imposes a trill of a major 2nd onto the input file. */

rtsetparams(44100, 2)
load("FLANGE")

rtinput("any.snd")

outskip = 0
inskip = 0
dur = DUR()
amp = 0.5

resonance = 1.0           /* how "ringy" are trill pitches? */
lowpitch = 8.00           /* lower pitch of major 2nd */
moddepth = 11.5           /* somehow makes a major 2nd above low pitch  ;-) */
modspeed = 6.0            /* speed of trill */
wetdrymix = 0.3           /* how prominent is trill? */

/* make an "ideal" square wave, but with smoothed ramps */
gsz = 20000    /* number of array slots */
makegen(2,7,gsz, 0,1000, 1,(gsz-4000)/2, 1,2000, -1,(gsz-4000)/2, -1,1000, 0)
fplot(2)

maxdelay = 1.0 / cpspch(lowpitch)

FLANGE(outskip, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix)
