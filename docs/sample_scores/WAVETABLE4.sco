// What makes the pitch go down sometimes?  Uncomment the printf for a hint.

print_off()
rtsetparams(44100, 2)
load("WAVETABLE")

maxamp = 3000
amp = maketable("linebrk", "nonorm", 1000, 0, 50, maxamp, 900, maxamp, 50, 0)
wavet = maketable("wave", 5000, 1, 0.3, 0.2)

start = 0.0
freq = 149.0
dur = 0.15

for (i = 0; i < 3000; i += 1) {
	//printf("freq: %f\n", freq)
	WAVETABLE(start, dur, amp, freq, pan=.5, wavet)
	start += 0.01
	freq += 25
}

