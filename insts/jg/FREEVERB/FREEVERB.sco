/* FREEVERB - a reverberator

   This reverb instrument is based on Freeverb, by Jezar
   (http://www.dreampoint.co.uk/~jzracc/freeverb.htm).

   p0  = output start time
   p1  = input start time
   p2  = input duration
   p3  = amplitude multiplier
   p4  = room size (0-1.07143 ... don't ask)
   p5  = pre-delay time (time between dry signal and onset of reverb)
   p6  = ring-down duration
   p7  = damp (0-100%)
   p8  = dry signal level (0-100%)
   p9  = wet signal level (0-100%)
   p10 = stereo width of reverb (0-100%)

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.  The curve is applied to the input sound *before*
   it enters the reverberator.

   If you enter a room size greater than the maximum, you'll get the
   maximum amount -- which is probably an infinite reverb time.

   Input can be mono or stereo; output can be mono or stereo.

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
dry = 40
wet = 30
width = 100

setline(0,1, 9,1, 10,0)

FREEVERB(outskip, inskip, dur, amp, roomsize, predelay, ringdur,
         damp, dry, wet, width)

