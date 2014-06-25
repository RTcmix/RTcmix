/*
   p0 = output start tim
   p1 = duration
   *p2 = pitch (Hz or oct.pc)
   *p3 = amplitude
   *p4 = wavetable vector guide [0-1]
   *p5 = pan [0-1]
   p6... pn = wavetables
*/

rtsetparams(44100, 2)
load("./libVWAVE.so")

srand(14)

// initialize array
theWs = { 1, 2, 3, 4, 5, 6, 7, 8 }

amp = 10000

for (i = 0; i < 8; i += 1) {
	theWs[i] = maketable("wave", 1000, random(), random(), random(), random())
}
vec = makeLFO("sine", 2, 0, 1)
pcurve = maketable("line", "nonorm", 1000, 0, 0, irand(1, 5), 0.005,
					irand(5, 10), 0.02)
VWAVE(0, 8.7, 7.00+pcurve, amp, vec, 0, theWs[0], theWs[1],
					theWs[2], theWs[3], theWs[4], theWs[5], theWs[6], theWs[7])



for (i = 0; i < 8; i += 1) {
	theWs[i] = maketable("wave", 1000, random(), random(), random(), random(),
					random(), random(), random())
}
vec = makeLFO("sine", 0.3, 0, 1)
pcurve = maketable("line", "nonorm", 1000, 0, 0, irand(1, 5), 0.005,
					irand(5, 10), 0.02)
VWAVE(0.1, 8.7, 7.00+pcurve, amp, vec, 1, theWs[0], theWs[1],
					theWs[2], theWs[3], theWs[4], theWs[5], theWs[6], theWs[7])


ampenv = maketable("window", 1000, "hanning")
for (i = 0; i < 8; i += 1) {
	theWs[i] = maketable("wave", 1000, random(), random(), random(), random())
}
vec = makeLFO("sine", 0.2, 0, 1)
pcurve = maketable("line", "nonorm", 1000, 0, 0, irand(1, 5), 0.005,
					irand(5, 10), 0.02)
VWAVE(3, 8.7, 6.07+pcurve, amp*ampenv, vec, 1, theWs[0], theWs[1],
					theWs[2], theWs[3], theWs[4], theWs[5], theWs[6], theWs[7])



for (i = 0; i < 8; i += 1) {
	theWs[i] = maketable("wave", 1000, random(), random(), random(), random(),
					random(), random(), random())
}
vec = makeLFO("sine", 4, 0, 1)
pcurve = maketable("line", "nonorm", 1000, 0, 0, irand(1, 5), 0.005,
					irand(5, 10), 0.02)
VWAVE(3.14, 8.7, 6.07+pcurve, amp*ampenv, vec, 0, theWs[0], theWs[1],
					theWs[2], theWs[3], theWs[4], theWs[5], theWs[6], theWs[7])
