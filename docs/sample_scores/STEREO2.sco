rtsetparams(44100, 2)
load("STEREO")

rtinput("../../snd/input.wav")
filedur = DUR()

amp = maketable("line", 1000, 0,0, .1, 1, 2,0)

// to make sure these very short notes are enveloped precisely
reset(10000)

dur = 1	// must init before loop, because we use it in for() statement
for (start = 0; start < 10.0; start += dur) {
	dur = random() * 0.2
	inskip = random() * (filedur - dur)
	STEREO(start, inskip, dur, amp, random(), -1)
}

