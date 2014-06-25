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
load("./libPVOC.so")
rtinput("/home/doug/sounds/11-English-Spoken-p.wav");
set_filter("./libPVRandBend.so");
fft = 512;
init_filter(fft, 300);

PVOC(start=0,inputskip=0,inputread=30,amp=1,inputchan=0,fft,window=2*fft,readin=64,putout=4*readin,pmult=0,npoles=0)
