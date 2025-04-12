if (?outfile) {
	outputfile = $outfile;
    set_option("audio_off");
	rtsetparams(44100, 4, 256);
    rtoutput(outputfile, "24");
}
else {
	rtsetparams(44100, 4, 256)
}

load("QMOVE")
rtinput("../../../snd/loocher.aiff");


bus_config("QMOVE","in0","aox0-7")
bus_config("QRVB","aix0-7","out0-3")

//mikes(45,0.5)

dist_front=120
dist_right=135
dist_rear=-147
dist_left=-111
height=100
rvbtime=3.1
abs_fac=2
space(dist_front,dist_right,dist_rear,dist_left,height,abs_fac,rvbtime)


insk=0
outsk=0
amp = 1

dur = 1;
dist_mikes = 12
inchan = 0

mindist = 12
maxdist = 80

set_attenuation_params(mindist, maxdist, 1.0);
threshold(0.01);
//reset(44100);

setline(0,1,2,1,3,0);

QMOVE(outsk,insk,dur,amp,xpos=-50,ypos=35,-dist_mikes,inchan);
outsk += 3;
QMOVE(outsk,insk,dur,amp,xpos=50,ypos=35,-dist_mikes,inchan);
outsk += 3;
QMOVE(outsk,insk,dur,amp,xpos=50,ypos=-35,-dist_mikes,inchan);
outsk += 3;
QMOVE(outsk,insk,dur,amp,xpos=-50,ypos=-35,-dist_mikes,inchan);

QRVB(0, 0, dur+7+rvbtime+0.5, 0.1);
