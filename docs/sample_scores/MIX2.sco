rtsetparams(44100, 2)

rtinput("../../snd/input.wav")
filedur = DUR()

amp = 1.0
env = maketable("line", 1000, 0,0, .2,1, 2,0)

// to make sure these very short notes are enveloped precisely
control_rate(10000)

dur = 1
for (outsk = 0; outsk < 14.0; outsk += dur) {
	insk = random() * filedur
	dur = random() * 0.2

	if (random() > 0.5)
		ch1 = 0
	else
		ch1 = 1
	if (random() > 0.5)
		ch2 = 0
	else
		ch2 = 1

	MIX(outsk, insk, dur, amp * env, ch1, ch2)
}

