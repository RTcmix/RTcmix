rtsetparams(44100, 8);
bus_config("MIX", "in0", "out0-7");
rtinput("input.wav");
setline(0,0,1,1,5,1,6,0);
MIX(0, 0, 3, 1, 0, 1, 0, 0, 0, 0, 0, 0);


