rtsetparams(44100, 2)
load("FMINST")

print_off()

notedur = 0.5

maxamp = 5000
amp = maketable("linebrk", "nonorm", 1000, 0, 500, maxamp, 500, 0)

wavetable = maketable("wave", 1000, "sine")
guide = maketable("line", 1000, 0,1, 2,0)

freq = 8.00
for (start = 0; start < 60; start = start + 0.1) {
	pan = random()
	FMINST(start, notedur, amp, freq, 179, 0, 10, pan, wavetable, guide)
	freq = freq + 0.002
}

