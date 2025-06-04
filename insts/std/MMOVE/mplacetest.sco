rtsetparams(44100,2,4096)
rtinput("../../../snd/nucular.wav")
load("MPLACE");
bus_config("MPLACE","in0","aox0-3")
bus_config("RVB","aix0-3","out0-1")

mikes(45,0.5)

dist_front=150
dist_right=130
dist_rear=-145
dist_left=-178
height=100
rvbtime=2
abs_fac=3
space(dist_front,dist_right,dist_rear,dist_left,height,abs_fac,rvbtime)

insk=0
outsk=0
dur=DUR(0);
pre_amp=120
dist_mikes=-5	// cartesian
//dist_mikes=-0.67	/* binaural cartesian */
inchan=0

MPLACE(outsk,insk,dur,pre_amp,0,dist_front-4,dist_mikes,inchan);

RVB(0, 0, dur+rvbtime, 0.9);

