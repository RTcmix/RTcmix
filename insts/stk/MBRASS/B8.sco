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
pch = 9.00
dur = 1.0

/*9.00*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 255, 480, 0.25)
start = start + dur + 0.1
pch = pch + 0.01

/*9.01*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 240, 490, 0.3)
start = start + dur + 0.1
pch = pch + 0.01

/*9.02*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 228, 530, 0.3)
start = start + dur + 0.1
pch = pch + 0.01

/*9.03*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 215, 550, 0.35)
start = start + dur + 0.1
pch = pch + 0.01

/*9.04*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 202, 590, 0.4)
start = start + dur + 0.1
pch = pch + 0.01

/*9.05*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 190, 640, 0.4)
start = start + dur + 0.1
pch = pch + 0.01

/*9.06*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 180, 670, 0.45)
start = start + dur + 0.1
pch = pch + 0.01

/*9.07*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 170, 710, 0.5)
start = start + dur + 0.1
pch = pch + 0.01

/*9.08*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 160, 740, 0.55)
start = start + dur + 0.1
pch = pch + 0.01

/*9.09*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 150, 780, 0.6)
start = start + dur + 0.1
pch = pch + 0.01

/*9.10*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 142, 840, 0.6)
start = start + dur + 0.1
pch = pch + 0.01

/*9.11*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 134, 860, 0.65)
start = start + dur + 0.1
pch = pch + 0.01

/*10.00*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 127, 940, 0.7)
start = start + dur + 0.1
pch = pch + 0.01
