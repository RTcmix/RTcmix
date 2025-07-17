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
rtinput("impulse.wav");


bus_config("QMOVE","in0","aox0-7")
bus_config("QRVB","aix0-7","out0-3")

mikes(45,0.5)

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
amp = 100

dur = DUR();
inchan = 0

// If dist-to-mike was 1.0, the sides of the square would be 1/sqrt(2)
// If dist-to-mike was sqrt(2), sides of square would be 1.0

/*
                                    20
                          x--------------------x
                          |                    |
                          |                    |
                          |                    |
                          |       {0,0}        |
                          |         *--------->|
                          |             10     |
                          |                    |
                          |                    |
                          |                    |
                          x--------------------x

*/

// TEST DIMENSIONS
dist_mikes = 10 * sqrt(2);	
// box is 20 x 20, so listener is at 0,0 and box corners are -10,10  10,10  10,-10  -10,-10

mindist = 12
maxdist = 80

//set_attenuation_params(mindist, maxdist, 1.0);
threshold(0.1);
//reset(44100);

QMOVE(outsk,insk,dur,amp,rho=20,theta=30,dist_mikes,inchan);
//QMOVE(outsk+dur,insk,dur,amp,xpos=0,ypos=20,-dist_mikes,inchan);
/*
QMOVE(outsk,insk,dur,amp,xpos=-50,ypos=35,-dist_mikes,inchan);
outsk += 2;
QMOVE(outsk,insk,dur,amp,xpos=50,ypos=35,-dist_mikes,inchan);
outsk += 2;
QMOVE(outsk,insk,dur,amp,xpos=50,ypos=-35,-dist_mikes,inchan);
outsk += 2;
QMOVE(outsk,insk,dur,amp,xpos=-50,ypos=-35,-dist_mikes,inchan);
*/

QRVB(0, 0, 2+rvbtime, 0.5);
