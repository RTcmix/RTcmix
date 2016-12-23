rtsetparams(44100, 2)
load("./libPINK.so")
load("NOISE")
load("FILTERBANK")

bus_config("PINK","aux 0-1 out")
bus_config("NOISE","aux 0-1 out")
bus_config("FILTERBANK","aux 0-1 in","out 0-1")

// Compare filtered white and pink noise from 50 to 4000 Hz

cf = maketable("line", "nonorm", 1000, 0,50, 1,4000)

NOISE(0,10,15000,0.5)
FILTERBANK(0,0,10,1,0,0,0.5, cf,0.0002,1)

PINK(11,10,15000,0.5)
FILTERBANK(11,0,10,1,0,0,0.5, cf,0.0002,1)
