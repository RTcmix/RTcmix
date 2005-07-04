rtsetparams(44100, 2)
load("FMINST")

print_off()

notedur = 1.5
amp = 3000
env = maketable("linebrk", 1000, 0, 500, 1, 500, 0)
pan = 0.5
wavetable = maketable("wave", 1000, "sine")
guide = maketable("line", "nonorm", 1000, 0,1, 2,0)

nfms = 1
for (start = 0; start < 60; start = start + 1.5) {
	freq = 8.00
	for (n = 0; n < nfms; n = n + 1) {
		FMINST(start, notedur, amp * env, freq, modfreq=179, min=0, max=10,
			pan, wavetable, guide)
		freq = freq + 0.02
	}
	nfms += 1
}

