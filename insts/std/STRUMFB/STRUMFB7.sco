// Translation of STRUM7.sco  -JGG

rtsetparams(44100, 2, 256)
load("STRUMFB")

dur = 7
pitch = 7.07
decaytime = 1
nyqdecaytime = 1
distgain = 10
fbgain = 0.05
fbpitch = 7.00
cleanlevel = 0
distlevel = 1
amp = 10000
squish = 2

viblow = 4
vibhigh = 7
vibdepth = 5 // Hz
vibseed = 0
lfreq = makerandom("low", frq=10, viblow, vibhigh, vibseed)
vibenv = maketable("line", 1000, 0,0, 1,1, 2,0)
vib = makeLFO("sine", lfreq, vibdepth) * vibenv
freq = cpspch(pitch) + vib

env = maketable("line", 1000, 0,1, 19,1, 20,0)

reset(500)

STRUMFB(0, dur, amp * env, freq, fbpitch, squish, decaytime, nyqdecaytime,
	distgain, fbgain, cleanlevel, distlevel)

