rtsetparams(44100, 2)
load("FILTERBANK")

rtinput("../../snd/loocher.aiff")
inchan = 0
inskip = 0
dur = DUR()
ringdur = 5

amp = 0.1
bw = maketable("line", "nonorm", 1000, 0,0.01, dur,0.001, dur+ringdur,0.001)

start = 0
FILTERBANK(start, inskip, dur, amp, ringdur, inchan, pan=0.1,
	cf=9.02, bw, g=1,
	cf=9.09, bw, g=1,
	cf=10.01, bw, g=1,
	cf=10.06, bw, g=1,
	cf=11.02, bw, g=1)
FILTERBANK(start, inskip, dur, amp, ringdur, inchan, pan=0.9,
	cf=8.02, bw, g=1,
	cf=9.05, bw, g=1,
	cf=10.03, bw, g=1,
	cf=10.10, bw, g=1,
	cf=11.04, bw, g=1)

