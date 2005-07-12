print_off()

rtsetparams(44100, 2)
load("FMINST")
load("WAVETABLE")

control_rate(2000)

env = maketable("linebrk", 1000, 0, 500, 1, 500, 0)
wave = maketable("wave", 1000, "sine")
guide = maketable("line", "nonorm", 1000, 0,1, 2,0)

amp = 6000 * env
freq = 8.00
for (start = 0; start < 20; start += 0.5) {
	pan = random()
	FMINST(start, dur=0.5, amp, freq, modfreq=179, 0, 10, pan, wave, guide)
	freq += 0.002
}


env = maketable("line", 1000, 0,1, 1,0)
amp = 4000 * env

wave = maketable("wave", 2000, 1, 0.3, 0.2)

srand(0)

for (start = 0; start < 20; start += 0.14) {
	freq = (random() * 200) + 35
	for (i = 0; i < 3; i += 1) {
		WAVETABLE(start, 0.4, amp, freq, 0, wave)
		WAVETABLE(start+random()*0.1, 0.4, amp, freq+(random()*7), 1, wave)
		if (start > 10) {
			wave = maketable("wave", 2000, 1, random(), random(), random(),
						random(), random(), random(), random(), random(),
						random(), random(), random())
		}
		freq += 125
	}
}

