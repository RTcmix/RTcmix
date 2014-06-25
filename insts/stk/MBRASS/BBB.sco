rtsetparams(44100, 2)
load("./libMBRASS.so")
load("WAVETABLE")

/* MBRASS - the "Brass" physical model instrument in Perry Cook/Gary Scavone's
	"stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = slide length (samps)
   p5 = lip filter (Hz)
   p6 = max pressure (0.0 - 1.0, I think...)
   p7 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note. (Try gen 18.)
   Or you can just call setline. If no setline or function table 1, uses
   flat amplitude curve.
*/

makegen(1, 24, 1000, 0,0, 3,1, 10,0)
makegen(2, 10, 1000, 1.0, 0.2, 0.3)

start = 0.0
pch = 10.00
dur = 1.0

x = 200
for(i = 0; i < 20; i=i+1)
{
	WAVETABLE(start, dur, 10000, pch, 0.5)
	start = start + dur + 0.1
	MBRASS(start, dur, 20000, cpspch(pch), 127, 940, 0.7)
	start = start + dur + 0.1
	x = x + 1
}


