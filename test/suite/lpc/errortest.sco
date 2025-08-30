/************   LPCPLAY test score *****************/
include tables.info

rtsetparams(44100, 2, 256);

load("LPCPLAY");
bus_config("LPCPLAY", "in 0", "out 0");

/* LPCPLAY arguments: */
/* p0=start,p1=dur,amp,p2=amp,p3=8ve.pch,p4=frame1,p5=frame2,p6=warp,p7=cf,p8=bw, p09/10 --> additional pitch specifications */

float thresh,randamp,fps,frame1,frame2,warp,cf,bw,dur,amp,start
float buzthresh, noisethresh;

fps = 44100/220.5
warp=0 
bw=0
cf=0
amp=6
frame1=0

autocorrect(true);

frameCount = dataset("./errortest.lpc",0)
frame2=frameCount-1
dur= 0.5 * (frame2-frame1)/fps

//set_thresh(buzthresh = 0.09, noisethresh = 0.1);

// Pure noise
lpcstuff(thresh = .0001, randamp = .3, 0, 0,0,0)
LPCPLAY(start=0,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

lpcstuff(thresh = .001, randamp = .3, 0, 0,0,0)
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

// Slight bit of voiced
lpcstuff(thresh = .01, randamp = .3, 0, 0,0,0)
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

// Natural mix
lpcstuff(thresh = .1, randamp = .3, 0, 0,0,0)
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

// Completely voiced
lpcstuff(thresh = 1, randamp = .3, 0, 0,0,0)
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);



lpcstuff(thresh = .0001, randamp = .3, 0, 0,0,0)
set_thresh(buzthresh = 0.00001, noisethresh = .0001);
LPCPLAY(start=start+dur+3,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

lpcstuff(thresh = .005, randamp = .3, 0, 0,0,0)
set_thresh(buzthresh = 0.0005, noisethresh = .005);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

lpcstuff(thresh = .001, randamp = .3, 0, 0,0,0)
set_thresh(buzthresh = 0.0001, noisethresh = .001);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

// Some voiced

lpcstuff(thresh = .05, randamp = .3, 0, 0,0,0)
set_thresh(buzthresh = 0.005, noisethresh = .05);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

lpcstuff(thresh = .01, randamp = .3, 0, 0,0,0)
set_thresh(buzthresh = 0.001, noisethresh = .01);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

// Completely voiced

lpcstuff(thresh = .5, randamp = .3, 0, 0,0,0)
set_thresh(buzthresh = 0.05, noisethresh = .5);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

// Theoretical Best

lpcstuff(thresh = 0.06, randamp = .3, 0, 0,0,0)
set_thresh(buzthresh = 0.01, noisethresh = 0.06);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

