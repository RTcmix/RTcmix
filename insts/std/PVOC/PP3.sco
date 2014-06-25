/* PVOC: phase vocoder instrument
*
*  p0 = outskip
*  p1 = inskip
*  p2 = dur
*  p3 = input channel
*  p4 = amplitude
*  p4 = fft size
*  p5 = window size
*  p7 = decimation amount (readin)
*  p8 = interpolation amount (putout)
*  p9 = pitch multiplier
*  p10 = npoles
*  p11 = gain thresh for oscil bank
*
*/

rtsetparams(44100, 1, 512);
load("PVOC")
rtinput("../../../snd/input.wav");

/* PVOC(start, inskip, indur, amp, inchan, */
/*     fftsize, windowsize, decim, interp, pitch, nLpcoeffs, synthThresh); */

start = 0;
inskip = 0;
duration = DUR(0);
gain = 1;
inskip = 0;
fftsize = 2048;
winsize = 2048*2;
pitch = 1;
decim = 512;
interp = 512;
nLPCoeffs = 0;
thresh = 0;

PVOC(start, inskip, duration, gain, 0, fftsize, winsize, decim, interp, pitch, nLPCoeffs, thresh);

start = start + duration;
pitch = pitch * 0.8;
PVOC(start, inskip, duration, gain, 0, fftsize, winsize, decim, interp, pitch, nLPCoeffs, thresh);

start = start + duration;
pitch = pitch * 0.8;
PVOC(start, inskip, duration, gain, 0, fftsize, winsize, decim, interp, pitch, nLPCoeffs, thresh);

start = start + duration;
pitch = pitch * 0.8;
PVOC(start, inskip, duration, gain, 0, fftsize, winsize, decim, interp, pitch, nLPCoeffs, thresh);

