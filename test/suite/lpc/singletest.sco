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
amp=8
frame1=0

autocorrect(true);

frameCount = dataset("./errortest.lpc",0)
frame2=frameCount-1
dur=(frame2-frame1)/fps

//set_thresh(buzthresh = 0.09, noisethresh = 0.1);

// Theoretical Best

lpcstuff(thresh = 0.06, randamp = .3, 0, 0,0,0)
set_thresh(buzthresh = 0.01, noisethresh = 0.06);
LPCPLAY(start=0,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

