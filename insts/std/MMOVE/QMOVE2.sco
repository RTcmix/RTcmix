rtsetparams(44100, 4, 256)
load("QMOVE")
rtinput("/Users/dscott/Sounds/Andante.wav");

bus_config("QMOVE","in0","aox0-7")
bus_config("QRVB","aix0-7","out0-3")

mikes(45,0.5)

dist_front=100
dist_right=100
dist_rear=-100
dist_left=-100
height=100
rvbtime=1
abs_fac=5
space(dist_front,dist_right,dist_rear,dist_left,height,abs_fac,rvbtime)


insk=0
outsk=0
amp = 1

dur = DUR();
dist_mikes = 10
inchan = 0

xpos = makeconnection("mouse", "x", min=dist_left, max=dist_right, dflt=20, lag=90,
			"x pos", "feet", 2);

ypos = makeconnection("mouse", "y", min=dist_rear, max=dist_front, dflt=20, lag=90,
			"y pos", "feet", 2);

mindist = 10
maxdist = 100

set_attenuation_params(mindist, maxdist, 1.0);
threshold(0.001);

QMOVE(outsk,insk,dur,amp,xpos,ypos,-dist_mikes,inchan);
QMOVE(dur,insk,dur,amp,xpos,ypos,-dist_mikes,inchan);
QMOVE(dur*2,insk,dur,amp,xpos,ypos,-dist_mikes,inchan);

QRVB(0, 0, (dur*4)+rvbtime+0.5, 0.5);
