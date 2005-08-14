// Translation of STRUM5.sco  -JGG

rtsetparams(44100, 2)
load("STRUMFB")

dur = 7.5
amp = 10000

pitch = cpspch(6.08)
pitch2 = cpspch(7.00)
freq = maketable("line", "nonorm", 1000, 0,pitch, 4,pitch, 6,pitch2, 8,pitch)
reset(200)

decaytime = 1
nyqdecaytime = 1
distgain = 10
fbgain = 0.05
fbpitch = 7.00
squish = 2
cleanlevel = 0
distlevel = 1

STRUMFB(0, dur, amp, freq, fbpitch, squish, decaytime, nyqdecaytime,
	distgain, fbgain, cleanlevel, distlevel)

