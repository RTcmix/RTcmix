// This score plays a series of loud transients using WAVETABLE.  These
// flow into COMPLIMIT, which limits the peaks.   -JGG

rtsetparams(44100, 2)
load("COMPLIMIT")
load("WAVETABLE")

bus_config("WAVETABLE", "aux 0 out")
bus_config("COMPLIMIT", "aux 0 in", "out 0-1")

totdur = 10

amp = 70000		// normally this would clip
freq = 400
wavet = maketable("wave", 1000, "saw")

dur = 0.02
incr = dur * 20

for (st = 0; st < totdur; st += incr)
	WAVETABLE(st, dur, amp, freq, 0, wavet)

inskip = 0
ingain = 0
outgain = 0
attack = 0.01
release = 0.06
threshold = -10
ratio = 100		// limiting
lookahead = attack
windowlen = 128
detect_type = 0
bypass = 0

COMPLIMIT(0, inskip, totdur, ingain, outgain, attack, release, threshold,
          ratio, lookahead, windowlen, detect_type, bypass, inchan=0, pan=.5)

