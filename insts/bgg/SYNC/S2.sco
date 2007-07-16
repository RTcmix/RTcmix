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

amp = 8000
ampenv = maketable("line", 1000, 0,0, 1,1, 9,1, 10,0)

wave = maketable("wave", 1000, "saw7")

for (i = 0; i < 7; i += 1) {
	pchenv = maketable("line", "nonorm", 1000, 0, irand(5.065, 5.075),
				irand(1, 5), irand(5.065, 5.075),
				irand(5, 9), irand(5.065, 5.075), 10, 5.07)
	wdex = maketable("line", "nonorm", 1000, 0, irand(100, 500), irand(1, 3),
				irand(100, 500), irand(4, 7),
				irand(100, 500), irand(8, 10), irand(100, 500))

	SYNC(0, 14, pchenv, amp*ampenv, wdex, wave, random())
}

