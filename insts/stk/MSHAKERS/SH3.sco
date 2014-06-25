rtsetparams(44100, 2)
load("./libMSHAKERS.so")

/* MSHAKERS - the "Shakers" physical model instrument in
        Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = energy (0.0-1.0)
   p4 = decay (0.0-1.0)
   p5 = # of objects (0.0-1.0)
   p6 = resonance freq (0.0-1.0)
   p7 = instrument selection (0-22 -- see "instruments" file for listing)
   p8 = percent of signal to left output channel [optional, default is .5]

*/

MSHAKERS(0, 3.5, 4*20000, 0.9, 1.05, 0.5, 0.7, 1)

amp = maketable("line", 1000, 0,0, 1,1, 2,1, 2.1,0)
MSHAKERS(4, 3.5, amp*5*20000, 0.9, 1.05, 0.5, 0.7, 3)

energy = maketable("line", "nonorm", 1000, 0,0, 1,0.2, 2,0)
resonance = makeLFO("tri", 3.5, 0.3, 0.4)
MSHAKERS(8, 3.5, 5000, energy, 0.1, 0.5, resonance, 14)

decay = maketable("line", "nonorm", 1000, 0,1, 1,1.1, 2,0)
pan = maketable("line", 1000, 0,1, 1,0.3, 2,0.7, 3,0, 4,1, 5,0)
MSHAKERS(12, 3.5, 3*20000, 0.9, decay, 0.5, 0.7, 19, pan)

nobjs = maketable("line", 1000, 0,1, 1,0, 3,1)
energy = maketable("line", "nonorm", 1000, 0,0.1, 1,0.2, 2,0)
MSHAKERS(15, 5.5, 5000, energy, 0.1, nobjs, 0.7, 5)
