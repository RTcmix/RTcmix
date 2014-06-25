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

makegen(1, 24, 1000, 0,0, 5,1, 10,0)

/* amps */
makegen(-3, 2, 18, 30000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000, 20000)
/* pch */
makegen(-4, 2, 18, 7.05, 7.07, 7.10, 8.00, 8.02, 8.03, 8.05, 8.07, 8.09, 8.10, 9.00, 9.02, 9.03, 9.05, 9.07, 9.09, 9.10, 10.00)
/* slide length */
makegen(-5, 2, 18, 95, 80, 65, 50, 43, 40, 35, 30, 15, 10, 255, 228, 215, 190, 170, 150, 142, 127)
/* lip filter */
makegen(-6, 2, 18, 188, 207, 250, 265, 295, 320, 355, 395, 430, 450, 480, 530, 550, 640, 710, 780, 840, 940)
/* max pressure */
makegen(-7, 2, 18, 0.15, 0.2, 0.2, 0.2, 0.2, 0.2, 0.25, 0.3, 0.3, 0.3, 0.25, 0.3, 0.35, 0.4, 0.5, 0.6, 0.6, 0.7)


start = 0;
for(i = 0; i < 100; i=i+1)
{
	index = random() * 18
	amp = sampfunc(3, index)
	pch = sampfunc(4, index)
	slide = sampfunc(5, index)
	lip = sampfunc(6, index)
	pressure = sampfunc(7, index)
	dur = random()*1.0 + 1.7
	MBRASS(start, dur, amp*0.3, cpspch(pch), slide, lip, pressure, random())
	start = start + (random() * 0.2 + 0.1)
}


