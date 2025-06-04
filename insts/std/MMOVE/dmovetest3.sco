if (?outfile) {
	outputfile = $outfile;
    set_option("audio_off");
	rtsetparams(44100, 2, 256);
    rtoutput(outputfile, "24");
}
else {
	rtsetparams(44100, 2, 256)
}

load("DMOVE")
rtinput("impulse.wav");


bus_config("DMOVE","in0","aox0-3")
bus_config("RVB","aix0-3","out0-1")

//mikes(45,0.5)

dist_front=120
dist_right=135
dist_rear=-147
dist_left=-111
height=100
rvbtime=3.1
abs_fac=5
space(dist_front,dist_right,dist_rear,dist_left,height,abs_fac,rvbtime)


insk=0
outsk=0
amp = 1000

dur = DUR();
inchan = 0

// TEST DIMENSIONS
dist_mikes = 20;	// 10 feet on either side of listener

mindist = 12
maxdist = 80

//set_attenuation_params(mindist, maxdist, 1.0);
threshold(0.1);
//reset(44100);

DMOVE(outsk,insk,dur,amp,rho=20,theta=30,dist_mikes,inchan);

RVB(0, 0, 2+rvbtime, 0.3);
