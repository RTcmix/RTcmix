rtsetparams(44100, 2)
load("./libMBRASS.so")

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

makegen(1, 24, 1000, 0,0, 0.05,1, 3.0,3, 3.5,0)

start = 0.0
lip = 100
for (i = 0; i < 40; i=i+1)
{
	MBRASS(start, 0.4, 20000, cpspch(8.00), 400, lip, 0.2)
	start = start + 0.5
	lip = lip+10
}
