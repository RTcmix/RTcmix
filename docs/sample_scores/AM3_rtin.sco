set_option("record=on")
rtsetparams(44100, 2, 256)
load("AM")

rtinput("AUDIO")

amp = 1
env = maketable("line", 1000, 0,0, 0.1,1, 0.2,1, 0.3,0)
wavetable = maketable("wave", 1000, "sine")

for (start = 0; start < 15.0; start = start + 0.1) {
	freq = random() * 400.0
	AM(start, 0, dur=0.3, amp * env, freq, 0, pan=random(), wavetable)
}

