rtsetparams(44100, 2)
load("STEREO")

rtinput("../../snd/input.wav")
filedur = DUR()

setline(0,0, .1, 1, 2,0)

reset(10000)
dur = 1;
for (start = 0; start < 10.0; start = start + dur) {
	dur = random() * 0.2
	inskip = random() * (filedur - dur)
	STEREO(start, inskip, dur, 1, random(), -1)
}

