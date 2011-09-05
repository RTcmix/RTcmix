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
rtinput("../../../snd/input.wav");

// Resynthesize with oscill bank, at 0.5 the orig pitch

PVOC(0, 0, DUR(0), 1, 0, 1024, 2048, 100, 100, 0.5)

