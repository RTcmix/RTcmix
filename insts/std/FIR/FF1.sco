/* FIR: simple FIR filter instrument
*
*  p0 = outsk
*  p1 = insk
*  p2 = dur
*  p3 = amp
*  p4 = total number of coefficients
*  p5...  the coefficients (up to 99 fir coefficients)
*
*  p3 (amp) can receive updates.
*  mono input / mono output only
*/

rtsetparams(44100, 1)
load("FIR")
rtinput("../../../snd/nucular.wav")

dur = DUR()
env = maketable("curve", 1000, 0,0,2, dur/4,1,-4, dur,0);

FIR(0, 0, dur, env * 0.05,
32, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9,
0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9,
0.9)

FIR(dur + 0.3, 0, dur, env,
32, 0.9, -0.9,  0.9, -0.9,  0.9, -0.9,  0.9, -0.9, 0.9, -0.9,  0.9, -0.9,  0.9,
-0.9,  0.9, -0.9, 0.9, -0.9,  0.9, -0.9,  0.9, -0.9,  0.9, -0.9, 0.9, -0.9,
0.9, -0.9,  0.9, -0.9,  0.9, -0.9)
