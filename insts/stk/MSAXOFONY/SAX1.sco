rtsetparams(44100, 2)
load("./libMSAXOFONY.so")

/* MSAXOFONY - the "Saxofony" physical model instrument in
        Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = noise gain (0.0-1.0)
   p5 = max pressure (0.0-1.0)
   p6 = reed stiffness (0.0-1.0)
   p7 = reed aperture (0.0-1.0)
   p8 = blow position (0.0-1.0)
   p9 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is breathPressure (amplitude) curve for the note.
   Or you can just call setline. If no setline or function table 1, uses
   flat curve.
*/

makegen(1, 24, 1000, 0,1, 2,0)
MSAXOFONY(0, 3.5, 20000.0, 243.0, 0.2, 0.7, 0.5, 0.3, 0.6)
MSAXOFONY(4, 3.5, 20000.0, 149.0, 0.2, 0.7, 0.5, 0.3, 0.6)
