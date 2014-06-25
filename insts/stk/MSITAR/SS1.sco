rtsetparams(44100, 2)
load("./libMSITAR.so")

/* MSITAR - the "Sitar" physical model instrument in
        Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = pluck amp (0.0-1.0)
   p5 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note.
   Or you can just call setline. If no setline or function table 1, uses
   flat curve.
*/

makegen(1, 24, 1000, 0,1, 1,1)
MSITAR(0, 3.5, 10000, cpspch(8.00), 0.9)
MSITAR(4, 3.5, 10000, cpspch(8.07), 0.9)
