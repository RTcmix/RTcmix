/************   LPCPLAY.data *****************/
/* p0=start,p1=dur,p2=8ve.pch,p3=frame1,p4=frame2,p5=amp,p6=warp,p7=cf,p8=bw, p9/10 --> additional pitch specifications */

float thresh,randamp,fps,frame1,frame2,D,cf,bw,incr,start,amp
float buzthresh, noisethresh;


rtsetparams(44100, 1, 256);
load("LPCPLAY");
bus_config("LPCPLAY", "in 0", "out 0");


dataset("/mnt/D/Sounds/lpcfiles/tori02.lpc",0)
/* p0=threshold,p1=randamp,p2=unvoiced_at_norm_rate,p3=risetime,p4=decaytime,p5=gain_threshold */
lpcstuff(thresh = .09,	randamp = .1,	0, 0,0,0)
set_thresh(buzthresh = 0.09, noisethresh = 0.1);

fps = 44100/250


frame1=0
frame2=600
D=0 
bw=0
cf=0
amp=10
incr=(frame2-frame1)/fps

/* straightforward synthesis */
LPCPLAY(start=0,incr,transp = .00001,frame1,frame2,amp,D,cf,bw)

setdev(1)  /* very slight deviation about base pitch, flat result */
LPCPLAY(start=start+incr+1,incr,transp = 8,frame1,frame2,amp,D,cf,bw)

setdev(0)  /* back to normal deviation, slower, higher, raise formants */
LPCPLAY(start=start+incr+1,incr*1.5,transp = .08,frame1,frame2,amp,D=.2,cf,bw)

/* lower, slower, lower formants --sex change operation */
LPCPLAY(start=start+incr*1.5+1,incr*1.5,transp= -.12,frame1,frame2,amp,D=-.25,cf,bw)

/* even more */
LPCPLAY(start=start+incr*1.5+1,incr*1.5,transp= 6.00,frame1,frame2,amp,D=-.25,cf,bw)

/* distorted curve, some formant shift, speeding up slightly */
setdev(30)
LPCPLAY(start=start+incr*1.5+1,incr*.9,transp=.02,frame1,frame2,amp,D=-.1,cf,bw)

/* modify pitch curves */
setdev(0)
LPCPLAY(start=start+incr+1,incr*.9,transp=8,frame1,frame2,amp,D=0,cf,bw,frame1+50,8,frame1+100,7,frame1+150,7.05,frame2,9)

/* some whispered speech */
lpcstuff(thresh = -.01,	randamp = .1,	0,0,0,0)
set_thresh(0.9, 1);
LPCPLAY(start=start+incr+1,incr,transp=8,frame1,frame2,amp,D=0,cf,bw)

/* highpass whispered speech */
LPCPLAY(start=start+incr+1,incr,transp=8,frame1,frame2,amp,D=0,cf=5,bw=.1)

/* highpass whispered speech, shift formants */
LPCPLAY(start=start+incr+1,incr,transp=8,frame1,frame2,amp,D=-.3,cf=7,bw=.05)

/* andrews sisters */
lpcstuff(thresh = .09,	randamp = .1,	0, 0,0,0)
set_thresh(buzthresh, noisethresh);
setdev(15)
amp = 3
LPCPLAY(start=start+incr+1,incr,transp=.01,frame1,frame2,amp,D=0,cf=0,bw=0)
LPCPLAY(start             ,incr,transp=.05,frame1,frame2,amp,D=0,cf=0,bw=0)
LPCPLAY(start             ,incr,transp=.08,frame1,frame2,amp,D=0,cf=0,bw=0)
