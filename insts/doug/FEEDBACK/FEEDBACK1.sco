rtsetparams(44100, 1, 2048);

rtinput("../../../snd/nucular.wav");

load("TRANS");
load("EQ");
load("./libFEEDBACK.so");


bus_config("TRANS", "in 0", "aux 0 out");
bus_config("FBRECEIVE", "aux 1 out");	// no apparent input from RTcmix’s point of view
bus_config("MIX", "aux 0-1 in", "aux 2 out");
bus_config("EQ", "aux 2 in", "aux 3 out");
bus_config("FBSEND", "aux 3 in", "out 0");	// RTcmix does not know that this feeds back into FBSEND

dur = DUR() * 2;
bufindex = 0;

TRANS(0, 0, dur, gain=0.4, -1.0);
FBRECEIVE(0, dur*2, $fbgain, bufindex);
MIX(0, 0, dur*2, 1, 0);
EQ(0, 0, dur*2, gain=0.8, type="lowpass", inchan=0, pan=0, bypass=0, freq=500, Q=2)
FBSEND(0, 0, dur*2, 1, bufindex);

