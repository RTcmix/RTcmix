rtsetparams(44100, 2)
load("./libMBANDEDWG.so")

/* MBANDEDWG - the "BandedWG" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
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

makegen(1, 24, 1000, 0,1, 1,0)
makegen(2, 24, 1000, 0,1, 1,0)

makegen(-3, 2, 8, 8.00, 8.02, 8.04, 8.05, 8.07, 8.08, 8.10, 9.00)

st = 0.0
for (i = 0; i < 50; i = i+1)
{
	index = random() * 8
	pch = sampfunc(3, index)
	MBANDEDWG(st, 1.0, 10000, cpspch(pch), 0.3, 1, 0.5, 0, 0.0, 1.0, 0.0, random())
	st = st + 0.1
}

for (i = 0; i < 50; i = i+1)
{
	index = random() * 8
	pch = sampfunc(3, index)
	MBANDEDWG(st, 1.0, 10000, cpspch(pch), 0.3, 1, 0.5, 1, 0.0, 1.0, 0.0, random())
	st = st + 0.15
}

for (i = 0; i < 50; i = i+1)
{
	index = random() * 8
	pch = sampfunc(3, index)
	MBANDEDWG(st, 1.0, 10000, cpspch(pch), 0.3, 1, 0.5, 2, 0.0, 1.0, 0.0, random())
	st = st + 0.2
}


for (i = 0; i < 50; i = i+1)
{
	index = random() * 8
	pch = sampfunc(3, index)
	MBANDEDWG(st, 1.0, 10000, cpspch(pch), 0.3, 1, 0.5, 3, 0.0, 1.0, 0.0, random())
	st = st + 0.25
}
