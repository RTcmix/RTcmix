// Basic simple MIX test for sanity
rtsetparams(44100, 2);
bus_config("MIX", "in0-1", "out0-1");
rtinput("../../snd/nucular.wav");
setline(0,0,1,1,5,1,6,0);
MIX(0, 0, DUR(0), 1, 0, 1);


