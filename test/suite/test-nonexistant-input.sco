rtsetparams(44100, 2);
bus_config("MIX", "in0-1", "out0-1");
rtinput("i_dont_exist.snd");
setline(0,0,1,1,5,1,6,0);
MIX(0, 0, 3, 1, 0, 1);


