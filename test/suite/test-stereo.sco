// Basic simple STEREO test for sanity
rtsetparams(44100, 2);
load("STEREO");
bus_config("STEREO", "in0", "out0-1");
rtinput("./sinetone.wav");
rtoutput("test.snd");
setline(0,0,1,1,5,1,6,0);
STEREO(0, 0, DUR(0), 1, 1);		// Left
STEREO(1, 0, DUR(0), 1, 0.5);	// Center
STEREO(2, 0, DUR(0), 1, 0);		// Right
system("rm -f test.snd");


