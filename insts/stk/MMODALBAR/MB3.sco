rtsetparams(44100, 2)
load("./libMMODALBAR.so")

/* MMODALBAR - the "ModalBar" physical model instrument in
	Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = stick hardness (0.0-1.0)
   p5 = stick position (0.0-1.0)
   p6 = modal preset
	- Marimba = 0
	- Vibraphone = 1
	- Agogo = 2
	- Wood1 = 3
	- Reso = 4
	- Wood2 = 5
	- Beats = 6
	- Two Fixed = 7
	- Clump = 8
   p7 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note.
   Or you can just call setline. If no setline or function table 1, uses
   flat curve.
*/

makegen(1, 24, 1000, 0,1, 1, 1)

st = 0;
for (j = 0; j < 9; j = j+1)
{
	pos = 0.0
	for (i = 0; i < 10; i = i+1)
	{
		MMODALBAR(st, 1.0, 30000, 279.0, 0.35, pos, j)
		pos = pos + 0.1
		st = st + 0.5
	}
	MMODALBAR(st, 1.0, 30000, 243.0, 0.35, pos-0.01, j)
}
