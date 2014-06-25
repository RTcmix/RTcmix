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

rtsetparams(44100, 1, 512);
load("./libPVOC.so")
rtinput("../../../snd/input.wav");
set_filter("./libPVFilterTest.so");
init_filter(100, 0.5);

PVOC(start=0,inputskip=0,inputread=DUR(0),amp=1,inputchan=0,fft=1024,window=2*fft,readin=1024,putout=readin,pmult=1,npoles=0)
