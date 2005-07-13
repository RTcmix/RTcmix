rtsetparams(44100, 2, 256)
load("WAVY")

dur = 60

amp = 6000
env = maketable("line", 1000, 0,0, .1,1, dur-.1,1, dur,0)

expr = "a - b"

wavetA = maketable("wave", 5000, "sine")
wavetB = maketable("wave", 5000, "tri")

freqA = cpspch(8.00)
freqB = makeconnection("mouse", "y", min=100, max=2000, dflt=freqA, lag=80,
				"freqB")

phase_offset = makeconnection("mouse", "x", min=0, max=1, dflt=0, lag=80,
				"phase offset")

WAVY(0, dur, amp * env, freqA, freqB, phase_offset, wavetA, wavetB, expr)

