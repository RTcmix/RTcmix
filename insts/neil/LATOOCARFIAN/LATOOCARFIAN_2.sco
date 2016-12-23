rtsetparams(44100, 2)
load("./libLATOOCARFIAN.so")
srand()

dur = 0.5
amp = 10000
for (start = 0; start < 20; start += 0.5) {
	a = irand(1, 3)
	b = irand(1, 3)
	c = irand(0.5, 1.5)
	d = irand(0.5, 1.5)
	LATOOCARFIAN(start, dur, amp, a, b, c, d)
}
