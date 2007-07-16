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

rtsetparams(44100, 1)
load("./libHALFWAVE.so")

w1 = maketable("wave3", 1000, 0.5, 1, 0)
w2 = maketable("wave3", 1000, 0.5, 1, 180)

dex = maketable("line", 1000, 0,0, 1,1, 2,0.5)

HALFWAVE(0, 3.5, 8.00, 20000, w1, w2, dex)

