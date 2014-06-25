/* PVOC: phase vocoder instrument
*
*  p0 = outskip
*  p1 = inskip
*  p2 = dur
*  p3 = input channel
*  p4 = amplitude
*  p5 = fft size
*  p6 = window size
*  p7 = decimation amount (readin)
*  p8 = interpolation amount (putout)
*  p9 = pitch multiplier (0 => overlap-add, >0 => oscil bank)
*  p10 = npoles
*
*/

rtsetparams(44100, 1, 512);
load("PVOC")
rtinput("/home/dscott/sounds/fine4.wav");
PVOC(0, 0, DUR(0), 1, 0, 1024, 2048, 100, 100, 0.9)

