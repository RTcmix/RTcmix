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
pch = 7.00
dur = 1.0
/*7.00*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 100000, cpspch(pch), 103, 140, 0.045)
start = start + dur + 0.1
pch = pch + 0.01

/*7.01*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 100000, cpspch(pch), 103, 150, 0.05)
start = start + dur + 0.1
pch = pch + 0.01

/*7.02*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 80000, cpspch(pch), 103, 160, 0.07)
start = start + dur + 0.1
pch = pch + 0.01

/*7.03*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 50000, cpspch(pch), 103, 170, 0.1)
start = start + dur + 0.1
pch = pch + 0.01

/*7.04*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 50000, cpspch(pch), 100, 180, 0.1)
start = start + dur + 0.1
pch = pch + 0.01

/*7.05*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 30000, cpspch(pch), 95, 188, 0.15)
start = start + dur + 0.1
pch = pch + 0.01

/*7.06*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 30000, cpspch(pch), 90, 195, 0.15)
start = start + dur + 0.1
pch = pch + 0.01

/*7.07*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 80, 207, 0.2)
start = start + dur + 0.1
pch = pch + 0.01

/*7.08*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 75, 220, 0.2)
start = start + dur + 0.1
pch = pch + 0.01

/*7.09*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 70, 235, 0.2)
start = start + dur + 0.1
pch = pch + 0.01

/*7.10*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 65, 250, 0.2)
start = start + dur + 0.1
pch = pch + 0.01

/*7.11*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 55, 255, 0.2)
start = start + dur + 0.1
pch = pch + 0.01

/*8.00*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 50, 265, 0.2)
start = start + dur + 0.1
pch = pch + 0.01


