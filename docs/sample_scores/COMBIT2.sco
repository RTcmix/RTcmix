rtsetparams(44100, 2)
load("COMBIT")

rtinput("../../snd/nucular.wav")
filedur = DUR()

totaldur = 14.0
dur = 0.1
amp = 0.3
env = maketable("line", 1000, 0,0, 0.1,1, 1,0) 
rvbtime = 0.5

for (outsk = 0; outsk < totaldur; outsk += 0.1) {
	insk = random() * filedur
	freq = random() * 500 + 100
	COMBIT(outsk, insk, dur, amp * env, freq, rvbtime, inchan=0, pan=random())
}

