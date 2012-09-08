rtsetparams(44100,2,4096)
rtinput("../../../snd/nucular.wav")
load("MPLACE");
bus_config("MPLACE","in0","aox0-3")
bus_config("RVB","aix0-3","out0-1")

mikes(45,0.5)

dist_front=100
dist_right=130
dist_rear=-145
dist_left=-178
height=100
rvbtime=3
abs_fac=8
space(dist_front,dist_right,dist_rear,dist_left,height,abs_fac,rvbtime)

insk=0
outsk=0
dur=DUR(0);
pre_amp=120
dist2sound=60
angle_sound=90
dist_mikes=5
//dist_mikes=0.67	/* binaural */
inchan=0
MPLACE(outsk,insk,dur,pre_amp,dist2sound,angle_sound+180,dist_mikes,inchan);
MPLACE(outsk+(256/44100),insk,dur,pre_amp,dist2sound,angle_sound,dist_mikes,inchan);
MPLACE(outsk+0.5,insk,dur,pre_amp,dist2sound*1.3,angle_sound+270,dist_mikes,inchan);

RVB(0, 0, dur+rvbtime+0.5, 1);

