rtsetparams(44100, 2)
load("./libMBOWED.so")

/* MBOWED - the "Bowed" physical model instrument in
        Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = vibrato freq low (Hz)
   p5 = vibrato freq high (Hz)
   p6 = vibrato depth (% of base frequency [decimal notation 0.1 == 10%])
   p7 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is breathPressure (amplitude) curve for the note.
   Or you can just call setline. If no setline or function table 1, uses
   flat curve.

   function table 2 = bow pressure tracking curve (0.0-1.0)
   function table 3 = bow position tracking curve (0.0-1.0)
   function table 4 = vibrato waveform
*/

makegen(1, 24, 1000, 0,0, 1,1, 2,0)
makegen(2, 24, 1000, 0,1, 1,1)
makegen(3, 24, 1000, 0,1, 2,0, 3,1)
makegen(4, 10, 1000, 1)
MBOWED(0, 7, 20000, 287.0, 5, 7, 0.02)
