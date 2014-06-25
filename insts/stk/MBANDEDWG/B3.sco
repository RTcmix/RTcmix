rtsetparams(44100, 2)
load("./libMBANDEDWG.so")

/* MBANDEDWG - the "BandedWG" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start tim
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = strike position (0.0-1.0)
   p5 = pluck flag (0: no pluck, 1: pluck)
   p6 = max velocity (0.0-1.0, I think...)
   p7 = preset #
         - Uniform Bar = 0
         - Tuned Bar = 1
         - Glass Harmonica = 2
         - Tibetan Bowl = 3
   p8 = bow pressure (0.0-1.0) 0.0 == strike only
   p9 = mode resonance (0.0-1.0) 0.99 == normal strike
   p10 = integration constant (0.0-1.0) 0.0 == normal?
   p11 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude (velocity?) curve for the note.
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.

   Assumes function table 2 is velocity curve for the note (interacts with
   p8)
*/

makegen(1, 24, 1000, 0,1, 2,0)
makegen(2, 24, 1000, 0,1, 2,0)

st = 0.0
stpos = 0.0
for (i = 0; i < 20; i = i+1)
{
	MBANDEDWG(st, 1.0, 10000, cpspch(8.00), 0.3, 1, 0.8, 1, 0.3, stpos, 0.0, 0.5)
	st = st + 0.2
	stpos = stpos + 0.05
}
