/* PAN - simple mixing instrument that follows a pan curve

   p0 = output start time
   p1 = input start time
   p2 = input duration
   p3 = amplitude multiplier
   p4 = input channel [optional, default is 0]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   Function table 2 is the panning curve, described by time,pan pairs.
   <pan> is expressed as the percentage of signal to place in the left
   channel (as in the STEREO instrument), from 0 (0%) to 1 (100%).
   Use gen 24 to make the function table, since it ensures the range
   is [0,1].

   Example:

      makegen(2, 24, 1000, 0,1, 1,0, 3,.5)
      PAN(start=0, inskip=0, dur=3, amp=1, inchan=0)

   This will pan input channel 0 from left to right over the first second.
   Then the sound travels back to the center during the next 2 seconds.

   PAN uses "constant-power" panning to prevent a sense of lost power when
   the pan location moves toward the center.

   John Gibson (jgg9c@virginia.edu), 1/26/00.
*/
load("PAN")
rtsetparams(44100, 2)

rtinput("/snd/Public_Sounds/jimi3dSound.aiff")

setline(0,0, 2,1, 8,1, 10,0)
makegen(2, 24, 1000, 0,1, .5,.1, 1,0)

PAN(start=0, inskip=0, dur=DUR(), amp=1, inchan=0)

