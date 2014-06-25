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
pch = 8.00
dur = 1.0

/*8.00*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 50, 265, 0.2)
start = start + dur + 0.1
pch = pch + 0.01

/*8.01*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 45, 280, 0.2)
start = start + dur + 0.1
pch = pch + 0.01

/*8.02*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 43, 295, 0.2)
start = start + dur + 0.1
pch = pch + 0.01

/*8.03*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 40, 320, 0.2)
start = start + dur + 0.1
pch = pch + 0.01

/*8.04*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 40, 340, 0.25)
start = start + dur + 0.1
pch = pch + 0.01

/*8.05*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 35, 355, 0.25)
start = start + dur + 0.1
pch = pch + 0.01

/*8.06*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 30, 370, 0.25)
start = start + dur + 0.1
pch = pch + 0.01

/*8.07*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 30, 395, 0.3)
start = start + dur + 0.1
pch = pch + 0.01

/*8.08*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 25, 415, 0.3)
start = start + dur + 0.1
pch = pch + 0.01

/*8.09*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 15, 430, 0.3)
start = start + dur + 0.1
pch = pch + 0.01

/*8.10*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 10, 450, 0.3)
start = start + dur + 0.1
pch = pch + 0.01

/*8.11*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 10, 480, 0.2)
start = start + dur + 0.1

/*8.11*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 270, 450, 0.23)
start = start + dur + 0.1
pch = pch + 0.01

/*9.00*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 10, 520, 0.2)
start = start + dur + 0.1

/*9.00*/
WAVETABLE(start, dur, 10000, pch, 0.5)
start = start + dur + 0.1
MBRASS(start, dur, 20000, cpspch(pch), 255, 480, 0.23)
start = start + dur + 0.1


