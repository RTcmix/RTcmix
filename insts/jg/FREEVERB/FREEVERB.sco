/* FREEVERB - a reverberator

   This reverb instrument is based on Freeverb, by Jezar at Dreampoint
   (http://www.dreampoint.co.uk/~jzracc/freeverb.htm).

   p0  = output start time
   p1  = input start time
   p2  = input duration
   p3  = amplitude multiplier
   p4  = room size (0-1.07143 ... don't ask)
   p5  = pre-delay time  (time between dry signal and onset of reverb)
   p6  = ringdown duration
   p7  = damp (0-100%)
   p8  = dry (0-?)
   p9  = wet (0-?)
   p10 = width (0-100%)

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.  The curve is applied to the input sound *before*
   it enters the reverberator.

   Be careful with the dry and wet levels -- it's easy to get extreme
   clipping!

   John Gibson <johngibson@virginia.edu>, 2 Feb 2001
*/
rtsetparams(44100, 2)
load("FREEVERB")

rtinput("/tmp/clave.aif")

outskip = 0
inskip = 0
dur = DUR()
amp = .8
roomsize = 0.9
predelay = .03
ringdur = 3
damp = 70
dry = .4
wet = .3
width = 100

setline(0,1, 9,1, 10,0)

FREEVERB(outskip, inskip, dur, amp, roomsize, predelay, ringdur,
         damp, dry, wet, width)

