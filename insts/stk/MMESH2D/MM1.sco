rtsetparams(44100, 2)
load("./libMMESH2D.so")

/* MMESH2D - the "Mesh2D" physical model instrument in
        Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = # of X points (2-12)
   p4 = # of Y points (2-12)
   p5 = xpos (0.0-1.0)
   p6 = ypos (0.0-1.0)
   p7 = decay value (0.0-1.0)
   p8 = strike energy (0.0-1.0)
   p9 = percent of signal to left output channel [optional, default is .5]

   Assumes function table 1 is amplitude curve for the note.
   Or you can just call setline. If no setline or function table 1, uses
   flat curve.
*/

makegen(1, 24, 1000, 0,1, 1,1)

st = 0
for (i = 0; i < 150; i = i+1)
{
	nx = random() * 10 + 2
	ny = random() * 10 + 2
	MMESH2D(st, 0.5, 17000, nx, ny, random(), random(), random(), random(), random())
	st = st + 0.1
}
