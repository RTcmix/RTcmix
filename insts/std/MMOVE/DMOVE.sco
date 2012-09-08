rtsetparams(44100, 2, 256)
load("DMOVE")
rtinput("/Users/dscott/Sounds/Andante.wav");

bus_config("DMOVE","in0","aox0-3")
bus_config("RVB","aix0-3","out0-1")

mikes(45,0.5)

dist_front=100
dist_right=100
dist_rear=-100
dist_left=-100
height=100
rvbtime=3
abs_fac=8
space(dist_front,dist_right,dist_rear,dist_left,height,abs_fac,rvbtime)


insk=0
outsk=0
amp = 1

dur = 60
dist_mikes = 2
inchan = 0

xpos = makeconnection("mouse", "x", min=dist_left, max=dist_right, dflt=10, lag=90,
			"x pos", "feet", 2);

ypos = makeconnection("mouse", "y", min=dist_rear, max=dist_front, dflt=10, lag=90,
			"y pos", "feet", 2);

mindist = 10
maxdist = 100

set_attenuation_params(mindist, maxdist, 1.0);
threshold(0.005);

DMOVE(outsk,insk,dur,amp,xpos,ypos,-dist_mikes,inchan);

RVB(0, 0, dur+rvbtime+0.5, 0.5);
