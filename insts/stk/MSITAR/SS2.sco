rtsetparams(44100, 2)
load("./libMSITAR.so")

/* MSITAR - the "Sitar" physical model instrument in
        Perry Cook/Gary Scavone's "stk" (synthesis tookkit).

   p0 = output start time
   p1 = duration
   p2 = amplitude multiplier
   p3 = frequency (Hz)
   p4 = pluck amp (0.0-1.0)
   p5 = percent of signal to left output channel [optional, default is .5]
*/

MSITAR(0, 3.5, 30000, cpspch(8.00), 0.9)

amp = maketable("line", 1000, 0,0, 1,1, 2,0)
freq = makerandom("linear", 9.0, cpspch(8.065), cpspch(8.075))
MSITAR(4, 3.5, amp*20000, freq, 0.9, 0.0)
freq = makerandom("linear", 7.0, cpspch(8.065), cpspch(8.075))
MSITAR(4, 3.5, amp*20000, freq, 0.9, 1.0)

stramp = maketable("line", 1000, 0,1, 2,0)
pan = makeLFO("sine", 14, 0, 1)
MSITAR(8, 3.5, 20000, cpspch(7.07), 0.9, pan, stramp)
