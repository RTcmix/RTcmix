rtsetparams(44100,2,4096)
rtinput("/home/dscott/sounds/Track11-EnglishSpokenMP.wav")
load("MMOVE")
bus_config("MMOVE","in0","aox0-1")
bus_config("RVB","aix0-1","out0-1")

mikes(45,0.5)

dist_front=100
dist_right=130
dist_rear=-145
dist_left=-178
height=100
rvbtime=3
abs_fac=1
space(dist_front,dist_right,dist_rear,dist_left,height,abs_fac,rvbtime)

insk=0
outsk=0
dur=10;
pre_amp=4
dist2sound=60
angle_sound=90
dist_mikes=5		/* for normal */
//dist_mikes=0.67	/* for binaural */
post_amp=2
inchan=0
path(0,30,-90, 1, 10, 90);
MMOVE(insk,outsk,dur,pre_amp,dist_mikes,post_amp,inchan);

RVB(0, 0, dur+rvbtime+0.5, 0.1);

