rtsetparams(44100, 1)
load("WAVETABLE")
load("SHAPE")

bus_config("WAVETABLE", "aux 0 out")
bus_config("SHAPE", "aux 0 in", "out 0")

dur = 10
amp = 10000
freq = 200
makegen(2, 10, 2000, 1)
start = 0
WAVETABLE(start, dur, amp, freq)

setline(0,0, 9,1, 10,0)
makegen(2, 17, 1000, 0.9, 0.3, -0.2, 0.6, -0.7, 0.9, -0.1)
makegen(3, 25, 1000, 1)    /* bell curve */

min = 0; max = 3.0
SHAPE(start, inskip = 0, dur, amp = 0.5, min, max, 0)

