rtsetparams(44100, 2)
load("WAVETABLE")
load("SHAPE")

bus_config("WAVETABLE", "aux 0 out")
bus_config("SHAPE", "aux 0 in", "out 0-1")

dur = 20
freq = 60

amp = 20000
wavet = maketable("wave", 2000, 1, .3, .1)
WAVETABLE(start=0, dur, amp, freq, 0, wavet)

transferfunc = maketable("wave", 1000, "square13")

// sample-and-hold distortion index
speed = 7                              // sh changes per second
indexguide = makerandom("even", speed, min=0, max=1, seed=1)
indexguide = makefilter(indexguide, "smooth", lag=20)
minidx = 0.0
maxidx = 3.0

normfunc = maketable("curve", "nonorm", 1000, 0,1,-1, 1,.3)

amp = 0.8
env = maketable("line", 1000, 0,1, dur-.03,1, dur,0)

SHAPE(start, inskip=0, dur, amp * env, minidx, maxidx, normfunc, 0, 1,
	transferfunc, indexguide)

// vary distortion index for other channel
indexguide = makerandom("even", speed, min=0, max=1, seed + 1)
indexguide = makefilter(indexguide, "smooth", lag=20)
SHAPE(start, inskip=0, dur, amp * env, minidx, maxidx, normfunc, 0, 0,
	transferfunc, indexguide)

