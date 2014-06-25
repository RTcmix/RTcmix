/*
   p0 = output start time
   p1 = duration
   *p2 = pitch (reset frequency) (Hz or oct.pc)
   *p3 = amplitude
   *p4 = oscillator writing frequency
   p5 = oscillator writing wavetable
   *p6 = pan [optional; default is 0]
*/

rtsetparams(44100, 1)
load("./libSYNC.so")

ampenv = maketable("line", 1000, 0,0, 1,1, 9,1, 10,0)

wave = maketable("wave", 1000, "sine")
wdex = maketable("line", "nonorm", 1000, 0, 200, 1, 400, 2, 200)

SYNC(0, 3.5, 225, 20000*ampenv, wdex, wave)

