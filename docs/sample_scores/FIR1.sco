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
rtinput("/snd/pablo1.snd")

FIR(0, 0, 7.0, 0.1, 32, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9, 0.9)

FIR(8.0, 0, 9.8, 1.0, 32, 0.9, -0.9,  0.9, -0.9,  0.9, -0.9,  0.9, -0.9, 0.9, -0.9,  0.9, -0.9,  0.9, -0.9,  0.9, -0.9, 0.9, -0.9,  0.9, -0.9,  0.9, -0.9,  0.9, -0.9, 0.9, -0.9,  0.9, -0.9,  0.9, -0.9,  0.9, -0.9)
