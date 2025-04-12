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

mikes(45,0.5)

dist_front=120
dist_right=135
dist_rear=-147
dist_left=-111
height=100
rvbtime=3.1
abs_fac=1
space(dist_front,dist_right,dist_rear,dist_left,height,abs_fac,rvbtime)


insk=0
outsk=0
amp = 10

dur = DUR();
dist_mikes = 30
inchan = 0

mindist = 12
maxdist = 80

//set_attenuation_params(mindist, maxdist, 1.0);
threshold(0.001);
reset(44100);

QMOVE(outsk,insk,dur,amp,rho=32,theta=maketable("line", "nonorm", 10000,0,-90,1,90),dist_mikes,inchan);
outsk += dur+rvbtime;
QMOVE(outsk,insk,dur,amp,rho=32,theta=maketable("line", "nonorm", 10000,0,90,1,270),dist_mikes,inchan);

QRVB(0, 0, 2*(dur+rvbtime), 0.3);
