load("WAVETABLE")
rtsetparams(44100, 1)
makegen(1, 7, 1000, 0, 50, 1, 900, 1, 50, 0)
makegen(2, 10, 1000, 1, 0.3, 0.2)
print_off()

start = 0.0
freq = 149.0
dur = 0.15

for (i = 0; i < 3000; i = i+1) {
	WAVETABLE(start, dur, 2000, freq)
	start = start + 0.01
	freq = freq + 25
	}
