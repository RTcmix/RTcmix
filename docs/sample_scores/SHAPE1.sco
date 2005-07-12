rtsetparams(44100, 2)
load("WAVETABLE")
load("SHAPE")

bus_config("WAVETABLE", "aux 0 out")
bus_config("SHAPE", "aux 0 in", "out 0-1")

dur = 10
amp = 10000
freq = 200
wavet = maketable("wave", 2000, "sine")
start = 0
WAVETABLE(start, dur, amp, freq, 0, wavet)

amp = maketable("line", 1000, 0,0, 9,1, 10,0) * 0.5

transferfunc = maketable("cheby", "nonorm", 1000,
                         0.9, 0.3, -0.2, 0.6, -0.7, 0.9, -0.1)

min = 0; max = 3.0
indexguide = maketable("window", 1000, "hanning")    // bell curve

SHAPE(start, inskip = 0, dur, amp, min, max, 0, inchan=0, pan=0.5,
	transferfunc, indexguide)

