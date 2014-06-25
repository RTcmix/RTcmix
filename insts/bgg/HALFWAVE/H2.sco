/*
   p0 = output start time
   p1 = duration
   *p2 = pitch (Hz or oct.pc)
   *p3 = amplitude
   p4 = first ghald wavetable
   p5 = second half wavetable
   *p6 = wavetable mid-crossover point (0-1)
   *p7 = pan [optional; default is 0]
*/

rtsetparams(44100, 2)
load("./libHALFWAVE.so")

amp = 3000
ampenv = maketable("line", 1000, 0,0, 1,1, 9,1, 10,0)

dur = 9.8

w1 = maketable("wave3", 1000, 0.5, 1, 0, 1, 0.2, 0, 3, 0.1, 0)
w2 = maketable("wave3", 1000, 0.5, 1, 180, 4, 0.5, 180, 7, 0.1, 180)

basepitch = 7.00
for (i = 0; i < 49; i += 1) {
	dex = maketable("line", "nonorm", 1000, 0, random(), 1, random(),
							2, random(), 3, random())

	prange = 0.24
	poff = maketable("line", "nonorm", 1000, 0, irand(0, prange),
				0.5, irand(0, prange), 1.4, irand(0, prange),
				2.5, irand(0, prange), 3, irand(0, prange), 5, irand(0, prange))

	pan = maketable("line", "nonorm", 1000, 0, random(), 1, random(),
				2, random(), 3, random())

	HALFWAVE(0, dur, basepitch+poff, amp*ampenv, w1, w2, dex, pan)
}

