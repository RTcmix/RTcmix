rtsetparams(44100, 2)
load("WAVETABLE")
control_rate(44100)

totdur = 20
numsines = 400
maxamp = 12000 / numsines
freq = 220
freqincr = 40
incr = 0.05

amp = maketable("line", "nonorm", 1000, 0,0, 1,maxamp, 9,maxamp, 10,0)
wavet = maketable("wave", 32767, "sine")

start = 0
for (i = 0; i < numsines; i += 1) {
	dur = totdur - start
	WAVETABLE(start, dur, amp, freq, pan=0.5, wavet)
	freq += freqincr
	start += incr
}
