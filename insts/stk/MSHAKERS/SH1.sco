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

st = 0
numobjs = 0
for (i = 0; i < 20; i = i+1)
{
	MSHAKERS(st, 0.5, 20000, 0.8, 0.8, numobjs, 0.1, 0)
	numobjs = numobjs + 0.05
	st = st + 0.2
}

st = st + 0.5
energy = 0
for (i = 0; i < 20; i = i+1)
{
	MSHAKERS(st, 0.5, 20000, energy, 0.8, 0.5, 0.1, 0)
	energy = energy + 0.05
	st = st + 0.2
}

st = st + 0.5
decay = 0
for (i = 0; i < 20; i = i+1)
{
	MSHAKERS(st, 0.5, 20000, 0.8, decay, 0.5, 0.1, 0)
	decay = decay + 0.05
	st = st + 0.2
}

st = st + 0.5
resofreq = 0
for (i = 0; i < 20; i = i+1)
{
	MSHAKERS(st, 0.5, 20000, 0.8, 0.8, 0.5, resofreq, 0)
	resofreq = resofreq + 0.05
	st = st + 0.2
}
