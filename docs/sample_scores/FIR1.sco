/* FIR: simple fir filter instrument
*
*  p0 = outsk  
*  p1 = insk 
*  p2 = dur
*  p3 = amp
*  p4 = total number of coefficients
*  p5...  the coefficients (up to 99 fir coefficients)
*
*  (no amplitude control, does channel 0 of both input and output only)
*
*/

rtsetparams(44100, 1)
load("FIR")
rtinput("../../snd/nucular.wav")
dur = DUR()

FIR(0, 0, dur, 0.03, 32, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9)

FIR(dur + 1, 0, dur, 1.0, 32, 0.9, -0.9,  0.9, -0.9,  0.9, -0.9,  0.9, -0.9, 0.9, -0.9,  0.9, -0.9,  0.9, -0.9,  0.9, -0.9, 0.9, -0.9,  0.9, -0.9,  0.9, -0.9,  0.9, -0.9, 0.9, -0.9,  0.9, -0.9,  0.9, -0.9,  0.9, -0.9)
