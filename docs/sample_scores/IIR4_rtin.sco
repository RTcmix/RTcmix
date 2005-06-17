set_option("record=on")  // must do this before rtsetparams
rtsetparams(44100, 2)
load("IIR")

rtinput("AUDIO", "MIC")
inchan = 0

env = maketable("line", 1000, 0,0, 0.1,1, 0.2,0)

for (start = 0; start < 7.8; start += 0.1) {
	setup((random() * 2000.0) + 300.0, -0.005, 1)
	INPUTSIG(start, inskip=0, dur=0.2, env * amp=0.3, inchan, random())
}

