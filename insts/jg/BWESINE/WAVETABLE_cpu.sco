rtsetparams(44100, 2)
load("BWESINE")
load("WAVETABLE")
control_rate(44100)
usebwe = 0

numsines = 300
maxamp = 30000 / numsines
freq = 220
freqincr = 10
startincr = 0.1
extradur = 3.0
srand(1) // pan

amp = maketable("line", "nonorm", 1000, 0,0, 1,maxamp, 9,maxamp, 10,0)
wavet = maketable("wave", 32767, "sine")
bw = 0.0
phase = 0

totdur = numsines * startincr
start = 0
for (i = 0; i < numsines; i += 1) {
	dur = (totdur - start) + extradur
	if (usebwe)
		BWESINE(start, dur, amp, freq, bw, phase, pan=random(), wavet)
	else
		WAVETABLE(start, dur, amp, freq, pan=random(), wavet)
	freq += freqincr
	start += startincr
}
