/************   LPCPLAY example score *****************/
rtsetparams(44100, 1, 256);
load("LPCPLAY");
bus_config("LPCPLAY", "in 0", "out 0");

/* LPCPLAY arguments: */
/* p0=start,p1=dur,amp,p2=amp,p3=8ve.pch,p4=frame1,p5=frame2,p6=warp,p7=cf,p8=bw, p9/10 --> additional pitch specifications */

float thresh,randamp,fps,frame1,frame2,warp,cf,bw,dur,amp,start,amp
float buzthresh, noisethresh;

/* open the LPC file to be used for the resynthesis */
dataset("/mnt/D/Sounds/lpcfiles/tori02.lpc",0)

/* lpcstuff arguments: */
/* p0=threshold,p1=randamp,p2=unvoiced_at_norm_rate,p3=risetime,p4=decaytime,p5=gain_threshold */

lpcstuff(thresh = .09,	randamp = .1,	0, 0,0,0)

set_thresh(buzthresh = 0.09, noisethresh = 0.1);

fps = 44100/250

frame1=0
frame2=600
warp=0 
bw=0
cf=0
amp=10

/* this calculation is just a trick to make 'dur' exactly equal to the */
/* time elapsed between frame1 and frame2 of the lpc data */

dur=(frame2-frame1)/fps

/* straightforward synthesis */
LPCPLAY(start=0,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw)

setdev(1)  /* very slight deviation about base pitch, flat result */
LPCPLAY(start=start+dur+1,dur,amp,transp = 8,frame1,frame2,warp,cf,bw)

setdev(0)  /* back to normal deviation, slower, higher, raise formants */
LPCPLAY(start=start+dur+1,dur*1.5,amp,transp = .08,frame1,frame2,warp=.2,cf,bw)

/* lower, slower, lower formants --sex change operation */
LPCPLAY(start=start+dur*1.5+1,dur*1.5,amp,transp= -.12,frame1,frame2,warp=-.25,cf,bw)

/* even more */
LPCPLAY(start=start+dur*1.5+1,dur*1.5,amp,transp= 6.00,frame1,frame2,warp=-.25,cf,bw)

/* distorted curve, some formant shift, speeding up slightly */
setdev(30)
LPCPLAY(start=start+dur*1.5+1,dur*.9,amp,transp=.02,frame1,frame2,warp=-.1,cf,bw)

/* modify pitch curves */
setdev(0)
LPCPLAY(start=start+dur+1,dur*.9,amp,transp=8,frame1,frame2,warp=0,cf,bw,frame1+50,8,frame1+100,7,frame1+150,7.05,frame2,9)

/* some whispered speech */
lpcstuff(thresh = -.01,	randamp = .1,	0,0,0,0)
set_thresh(0.9, 1);
LPCPLAY(start=start+dur+1,dur,amp,transp=8,frame1,frame2,warp=0,cf,bw)

/* highpass whispered speech */
LPCPLAY(start=start+dur+1,dur,amp,transp=8,frame1,frame2,warp=0,cf=5,bw=.1)

/* highpass whispered speech, shift formants */
LPCPLAY(start=start+dur+1,dur,amp,transp=8,frame1,frame2,warp=-.3,cf=7,bw=.05)

/* andrews sisters */
lpcstuff(thresh = .09,	randamp = .1,	0, 0,0,0)
set_thresh(buzthresh, noisethresh);
setdev(15)
amp = 3
LPCPLAY(start=start+dur+1,dur,amp,transp=.01,frame1,frame2,warp=0,cf=0,bw=0)
LPCPLAY(start             ,dur,amp,transp=.05,frame1,frame2,warp=0,cf=0,bw=0)
LPCPLAY(start             ,dur,amp,transp=.08,frame1,frame2,warp=0,cf=0,bw=0)
