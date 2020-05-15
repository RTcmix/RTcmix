rtsetparams(44100, 2, 256)
load("DMOVE")
rtinput("../../../snd/loocher.aiff");

bus_config("DMOVE","in0","aox0-3")
bus_config("RVB","aix0-3","out0-1")

mikes(45,0.5)

dist_front=50
dist_right=25.1
dist_rear=-10
dist_left=-35.2
height=100
rvbtime=1
abs_fac=5
space(dist_front,dist_right,dist_rear,dist_left,height,abs_fac,rvbtime)


insk=0
outsk=0
amp = 1

dur = DUR();
dist_mikes = 2
inchan = 0

// move the sound from your left out to your right near the front wall

xpos = maketable("line", "nonorm", 10000, 0, dist_left/2, 1, dist_right/2);

ypos = maketable("line", "nonorm", 10000, 0, 0, 1, dist_front);

mindist = 5
maxdist = 80

set_attenuation_params(mindist, maxdist, 1.0);
threshold(1/44100);
reset(44100);

DMOVE(outsk,insk,dur,amp,xpos,ypos,-dist_mikes,inchan);

RVB(0, 0, dur+rvbtime+0.5, 0.5);
