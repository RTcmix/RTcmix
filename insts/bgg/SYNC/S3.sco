/*
   p0 = output start time
   p1 = duration
   *p2 = pitch (reset frequency) (Hz or oct.pc)
   *p3 = amplitude
   *p4 = oscillator writing frequency
   p5 = oscillator writing wavetable
   *p6 = pan [optional; default is 0]
*/

rtsetparams(44100, 2)
load("./libSYNC.so")

reset(10000)

pches = { 0.00, 0.01, 0.03, 0.04, 0.06, 0.09, 0.12 }
lpches = len(pches)
bpitch = 7.09

amp = 10000
ampenv = maketable("line", 1000, 0,0, 1,1, 9,1, 10,0)

wave = maketable("wave", 1000, "saw3")
wdex = maketable("line", "nonorm", 1000, 0, 400, 1, 200)

st = 0
dur = 0.15
for (i = 0; i < 78; i += 1) {
	pch = bpitch + pches[trand(0, lpches)]
	SYNC(st, dur, pch, amp*ampenv, wdex, wave, random())
	st += irand(0, dur)
}

