/* PVOC: phase vocoder instrument
*
*  p0 = outskip
*  p1 = inskip
*  p2 = dur
*  p3 = amp
*  p4 = input channel
*  p5 = fft size
*  p6 = window size
*  p7 = decimation amount (readin)
*  p8 = interpolation amount (putout)
*  p9 = pitch multiplier
*  p10 = npoles
*
*/

rtsetparams(44100, 1, 1024);
load("PVOC")
rtinput("../../../snd/nucular.wav");
set_filter("./libPVTransBend.so");
fft = 512;

dur = DUR();
scale = 4
readin = 64
fftcalls = dur * SR() / readin;

init_filter(fftcalls, 0, 1.01, 2, -2.02);

PVOC(start=0,inputskip=0,inputread=dur*scale,amp=1,inputchan=0,fft,window=2*fft,readin,putout=readin*scale,pmult=0,npoles=0)
