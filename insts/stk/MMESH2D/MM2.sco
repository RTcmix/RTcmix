rtsetparams(44100, 2)
load("./libMMESH2D.so")

/* MMESH2D - the "Mesh2D" physical model instrument in
        Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = # of X points (2-12)
   p4 = # of Y points (2-12)
   p5 = xpos (0.0-1.0)
   p6 = ypos (0.0-1.0)
   p7 = decay value (0.0-1.0)
   p8 = strike energy (0.0-1.0)
   p9 = percent of signal to left output channel [optional, default is .5]
*/

MMESH2D(0, 4.5, 3*30000, 12, 11, 0.8, 0.9, 1.0, 1.0, 0.5)

amp = maketable("line", 1000, 0,0, 4,1, 5,0)
pan = makeLFO("tri", 0.5, 0.0, 1.0)
MMESH2D(5, 4.5, amp*100*30000, 10, 11, 0.7, 0.1, 1.0, 1.0, pan)
