rtsetparams(44100, 2)
load("./libDSPMATH.so")

rtinput("../../../snd/huhh.wav")

drive = maketable("line", 1000, 0, 0.5, 1, 8, 2, 1);

DSPMATH(0, 0, DUR(), gain=20000, "tanh", drive);
