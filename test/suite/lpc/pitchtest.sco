/************   LPCPLAY test score *****************/
include tables.info

rtsetparams(44100, 2, 256);

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

autocorrect(true);

frameCount = dataset("./48poles.lpc",0)

lpcstuff(thresh = .09, randamp = .3, 0, 0,0,0)
set_thresh(buzthresh = 0.09, noisethresh = 0.1);
frame2=frameCount-1
dur=(frame2-frame1)/fps


// Normal
LPCPLAY(start=0,dur,amp,transp = .00001,frame1,frame2,warp,cf,bw);

// Set transp to 400 Hz
LPCPLAY(start=start+dur+1,dur,amp,400,frame1,frame2,warp,cf,bw);

// Set transp via PCH
LPCPLAY(start=start+dur+1,dur,amp,6.07,frame1,frame2,warp,cf,bw);

// Fixed pitch
LPCPLAY(start=start+dur+1,dur,amp,-8.00,frame1,frame2,warp,cf,bw);

// Fixed PCH with vibrato
LPCPLAY(start=start+dur+1,dur,amp,-9.00+makeLFO("tri",4.3, -0.001, -0.012),frame1,frame2,warp,cf,bw);

// Fixed CPS with vibrato
LPCPLAY(start=start+dur+1,dur,amp,-200+makeLFO("tri",8.3, 50),frame1,frame2,warp,cf,bw);

// Transpose down 24 semitones over duration
LPCPLAY(start=start+dur+1,dur,amp,makeline({0,0.00,1,-0.24}),frame1,frame2,warp,cf,bw);
