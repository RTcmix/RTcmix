rtsetparams(44100, 2)
load("./libDSPMATH.so")

rtinput("../../../snd/input.wav")

DSPMATH(0, 0, DUR(), gain=10000, "tanh", drive=16);
