// Test of aux busses
set_option("audio_off");	// Because not all card support 4chan
rtsetparams(44100, 4);
load("TRANS");
load("STEREO");
bus_config("TRANS3", "in0", "aox0");			// Mono pipe
bus_config("TRANS", "in2", "aox2");				// Mono pipe
bus_config("STEREO", "in1", "in3", "aox2-3");	// Stereo pipe, non-contiguous
bus_config("MIX", "aix0-3", "out0-3");			// Combine all back together
rtinput("./4channel.wav");
dur = DUR(0);
TRANS(0, 0, dur, 1.0, 0.0, 1.0);
TRANS3(0, 0, dur, 1.0, 0.0, 1.0);
STEREO(0, 0, dur, 1.0, 0.5);
MIX(0, 0, DUR(0), 1, 0, 1, 2, 3);

