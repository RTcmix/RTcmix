/* FILTSWEEP - time-varying biquad filter

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = ring-down duration (time to ring out filter after input stops)
   p5 = sharpness (integer btw 1 and 5, inclusive) [optional, default is 1]
   p6 = balance output and input signals (0:no, 1:yes) [optional, default is 1]
   p7 = input channel [optional, default is 0]
   p8 = percent to left channel [optional, default is .5]

   p5 (sharpness) is just the number of filters to add in series. Using more
   than 1 decreases the actual bandwidth of the total filter. This sounds
   different from decreasing the bandwith of 1 filter using the bandwidth
   curve, described below. (Mainly, it further attenuates sound outside the
   passband.) If you don't set p6 (balance) to 1, you'll need to change p3
   (amp) to adjust for loss of power caused by connecting several filters
   in series. Guard your ears!

   p6 (balance) tries to adjust the output of the filter so that it has
   the same power as the input. This means there's less fiddling around
   with p3 (amp) to get the right amplitude (audible, but not ear-splitting)
   when the bandwidth is very low and/or sharpness is > 1. However, it has
   drawbacks: it can introduce a click at the start of the sound, it can
   cause the sound to pump up and down a bit, and it eats extra CPU time.

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   Function table 2 is the center frequency curve, described by time,cf pairs.
   Use gen 18.

   Function table 3 is the bandwidth curve, described by time,bw pairs.
   If bw is negative, it's interpreted as a percentage (from 0 to 1)
   of the cf. Use gen 18.

   John Gibson (jgg9c@virginia.edu), 2/5/00.
*/
rtsetparams(44100, 2)
load("FILTSWEEP")

rtinput("/snd/Public_Sounds/motorclip.snd")

inskip = 0
dur = DUR() - inskip

amp = .5
balance = 1
sharpness = 2
bw = -.3
ringdur = .5

reset(5000)
setline(0,0, 1,1, dur-1,1, dur,0)

/* left chan */
makegen(2,18,2000, 0,20, dur,4000)            /* increasing cf */
makegen(3,18,2000, 0,bw,  dur,bw)             /* constant bw */
FILTSWEEP(start=0, inskip, dur, amp, ringdur, sharpness, balance,
          inchan=0, pctleft=1)

/* right chan */
makegen(2,18,2000, 0,4000, dur,20)            /* decreasing cf */
makegen(3,18,2000, 0,bw,  dur,bw)             /* constant bw */
FILTSWEEP(start=.01, inskip, dur, amp, ringdur, sharpness, balance,
          inchan=1, pctleft=0)

