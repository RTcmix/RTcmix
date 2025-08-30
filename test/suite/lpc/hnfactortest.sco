/************   LPCPLAY test score *****************/
rtsetparams(44100, 1, 256);

load("LPCPLAY");
bus_config("LPCPLAY", "in 0", "out 0");

/* LPCPLAY arguments: */
/* p0=start,p1=dur,amp,p2=amp,p3=8ve.pch,p4=frame1,p5=frame2,p6=warp,p7=cf,p8=bw, p09/10 --> additional pitch specifications */

float thresh,randamp,fps,frame1,frame2,warp,cf,bw,dur,amp,start
float buzthresh, noisethresh;

fps = 44100/250
warp=0 
bw=0
cf=0
amp=10
frame1=0


frameCount = dataset("./48poles.lpc",0)

lpcstuff(thresh = .09, randamp = .1, 0, 0,0,0)
set_thresh(buzthresh = 0.09, noisethresh = 0.1);
frame2=frameCount-1
dur=(frame2-frame1)/fps

LPCPLAY(start=0,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw)

set_hnfactor(2);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw)

set_hnfactor(4);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw)

set_hnfactor(8);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw)

set_hnfactor(16);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw)

set_hnfactor(1);
LPCPLAY(start=start+dur+2,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw)

set_hnfactor(0.5);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw)

set_hnfactor(0.25);
LPCPLAY(start=start+dur+1,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw)
