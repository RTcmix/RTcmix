set_option("record=on")
rtsetparams(44100, 2, 256)
load("COMBIT")

rtinput("AUDIO")

dur = 0.1
env = maketable("line", 1000, 0,0, 0.1,1, 1,0) 

for (outsk = 0; outsk < 14.0; outsk = outsk + 0.1) {
	freq = random() * 500 + 100
	COMBIT(outsk, 0, dur, env * 0.1, freq, rvt=.5, 0, pan=random())
}

