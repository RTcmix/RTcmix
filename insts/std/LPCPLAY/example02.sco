/************   LPCIN.data *****************/

/* p0=start,p1=inskip,p2=dur,p4=amp,p5=frame1,p6=frame2[,p7=warp] */

rtsetparams(44100, 1, 256);
load("LPCPLAY");
bus_config("LPCIN", "in 0", "out 0");

rtinput("guitar.wav");

float thresh,frame1,frame2,warp,start,amp

frames = dataset("spoken.lpc",0)

/* none of this is currently used by LPCIN, but might be */

/* p0=threshold,p1=randamp,p2=unvoiced_at_norm_rate,p3=risetime,p4=decaytime,p5=gain_threshold */
lpcstuff(thresh = .01,	randamp = .1, 0,	0.1,0.5,0)
set_thresh(0.001, 0.1);

frame1=0
frame2=frames-1
warp=0
amp=0.3

LPCIN(start=0,inskip=0, DUR(0), amp, frame1,frame2,warp);
