rtsetparams(44100, 2)
load("WAVESHAPE")

masteramp = 2.0

dur = 7
pitch = 7.02
amp = 12000 * masteramp
env = maketable("line", 1000, 0,0, .1,0, 3.5,1, 7,0)
wavetable = maketable("wave", 1000, "sine")
transferfunc = maketable("cheby", 1000, 0.9, 0.3, -0.2, 0.6, -0.7)
indexguide = maketable("line", 1000, 0,0, 3.5,1, 7,0)

WAVESHAPE(st=0, dur, pitch, 0, 1, amp * env, pan=0.99,
	wavetable, transferfunc, indexguide)

pitch = 6.091
amp = 18000 * masteramp
env = maketable("line", 1000, 0,0, 1.5,1, 7,0)
indexguide = maketable("line", 1000, 0,1, 7,0)

WAVESHAPE(st=4, dur, pitch, 0, 1, amp * env, pan=0.01,
	wavetable, transferfunc, indexguide)

