rtsetparams(44100,2,1024)
load("WAVETABLE");
load("MMOVE");
rtoutput("mmovetest-out.wav", "24");
bus_config("WAVETABLE","aox0")
bus_config("MMOVE","aix0","aox1-4")
bus_config("RVB","aix1-4","out0-1")

mikes(45,0.5)

// get updates to happen as fast as possible
reset(44100);
threshold(1/44100.0);

dist_front=100
dist_right=130
dist_rear=-145
dist_left=-178
height=100
rvbtime=1
abs_fac=8
space(dist_front,dist_right,dist_rear,dist_left,height,abs_fac,rvbtime)

wavet = maketable("wave", 5000, 1, .5, 0, .25);

insk=0
outsk=0
dur=5
pre_amp=120

cpath(0, dist_left, 10, 1, dist_right, 10);
dist_mikes=2
//dist_mikes=0.67	/* binaural */
inchan=0
WAVETABLE(0, dur, 5000, 440, 0, wavet);
MMOVE(outsk,insk,dur,pre_amp,dist_mikes,inchan);
RVB(0, 0, dur+rvbtime+0.5, 0.01);

