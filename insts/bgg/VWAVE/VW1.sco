/*
   p0 = output start tim
   p1 = duration
   *p2 = pitch (Hz or oct.pc)
   *p3 = amplitude
   *p4 = wavetable vector guide [0-1]
   *p5 = pan [0-1]
   p6... pn = wavetables
*/

rtsetparams(44100, 1)
load("./libVWAVE.so")

w1 = maketable("wave", 1000, "sine")
w2 = maketable("wave", 1000, "square3")
w3 = maketable("wave", 1000, "saw")

vec = maketable("line", 1000, 0,0, 1,1, 2,0)

VWAVE(0, 3.5, 9.00, 20000, vec, 0, w1, w2, w3)
