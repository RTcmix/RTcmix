rtsetparams(44100, 2)
load("WAVESHAPE")

dur = 7

amp = 30000
env = maketable("line", 1000, 0,0, 3.5,1, 7,0)

wavetable = maketable("wave", 1000, "sine")
transferfunc = maketable("linebrk", "nonorm", 1000,
		-0.7, 200, -0.5, 300, 0, 300, 0.5, 200, 0.7)
indexguide = maketable("line", 1000, 0,0, 3.5,1, 7,0)

minindex = 0
maxindex = 1

WAVESHAPE(0, dur, pitch=7.02, minindex, maxindex, amp * env, pan = 0.7,
	wavetable, transferfunc, indexguide)
WAVESHAPE(0, dur, pitch=7.021, minindex, maxindex, amp * env, pan = 0.3,
	wavetable, transferfunc, indexguide)

