// Test of bus checking code
rtsetparams(44100, 1);
load("TRANS");
bus_config("TRANS3", "in0", "aox0");			// Mono pipe
bus_config("TRANS", "aix0", "aox1");			// Mono pipe
bus_config("MIX", "aix1", "aox0");			// Attempt to loop back!
rtinput("./input.wav");
dur = DUR(0);
TRANS(0, 0, dur, 1.0, 0.0, 0);
TRANS3(0, 0, dur, 1.0, 0.0, 0);
MIX(0, 0, DUR(0), 1, 0, 1, 2, 3);

