// Translation of STRUM6.sco  -JGG

rtsetparams(44100, 2)
load("STRUMFB")

dur = 13.5

pitch = cpspch(6.08)
pitch2 = cpspch(7.00)
freq1 = maketable("line", "nonorm", 6 * 100,
	0,pitch, 2,pitch, 4,pitch2, 6,pitch)

freq2 = maketable("linestep", "nointerp", "nonorm", 7.5 * 100,
	0,		cpspch(6.10),
	0.2,	cpspch(7.00),
	0.3,	cpspch(7.02),
	0.4,	cpspch(7.00),
	0.5,	cpspch(6.10),
	7.5,	cpspch(6.10))
freq = modtable(freq1, "concat", freq2)
//plottable(freq)

fbfreq = maketable("linestep", "nointerp", "nonorm", dur * 100,
	0.0,	cpspch(7.00),
	6.0,	cpspch(7.00),
	7.5,	cpspch(7.07),
	8.5,	cpspch(7.09),
	9.5,	cpspch(6.01),
	11.5,	cpspch(8.01),
	13.5,	cpspch(8.01))
//dumptable(fbfreq)
//plottable(fbfreq)

reset(100)

decaytime = 2.0	// NB: was 1 in STRUM6.sco, but 2 matches orig sound better
nyqdecaytime = 1
distgain = 10
fbgain = 0.05
cleanlevel = 0
distlevel = 1
amp = 10000
squish = 2

STRUMFB(0, dur, amp, freq, fbfreq, squish, decaytime, nyqdecaytime,
	distgain, fbgain, cleanlevel, distlevel)

