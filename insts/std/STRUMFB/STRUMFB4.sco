// Translation of STRUM4.sco  -JGG

rtsetparams(44100, 2)
load("STRUMFB")

dur = 7
freq = 6.08
decaytime = 1
nyqdecaytime = 1
distgain = 10
fbgain = 0.05
cleanlevel = 0
distlevel = 1
amp = 20000
squish = 2

start = 0
fbfreq = 7.00
STRUMFB(start, dur, amp, freq, fbfreq, squish, decaytime, nyqdecaytime,
	distgain, fbgain, cleanlevel, distlevel)

start = 8
fbfreq = 7.01
STRUMFB(start, dur, amp, freq, fbfreq, squish, decaytime, nyqdecaytime,
	distgain, fbgain, cleanlevel, distlevel)

start = 16
fbfreq = 6.01
STRUMFB(start, dur, amp, freq, fbfreq, squish, decaytime, nyqdecaytime,
	distgain, fbgain, cleanlevel, distlevel)
