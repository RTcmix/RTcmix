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
inst = 0
for (j = 0; j < 23; j = j+1)
{
	for (i = 0; i < 7; i = i+1)
	{
		MSHAKERS(st, 0.5, 20000, 0.9, 0.8, 0.5, 0.7, inst)
		st = st + 0.2
	}
	inst = inst + 1
}
