rtsetparams(44100, 2)

rtinput("../../snd/input.wav")
filedur = DUR()

amp = maketable("line", "nonorm", 1000, 0,0, 0.1,1, 2,0)

// to make sure these very short notes are enveloped precisely
control_rate(10000)

outsk = 0.0
for (i = 0; i < 550; i += 1) {
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

	MIX(outsk, insk, dur, amp, ch1, ch2)

	outsk += 0.05
}

