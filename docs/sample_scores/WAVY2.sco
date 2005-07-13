rtsetparams(44100, 2, 256)
load("WAVY")

dur = 30
amp = 10000
env = maketable("line", 1000, 0,0, .1,1, dur-.1,1, dur,0)
freqA = cpspch(6.00)

expr = "a * b"

// try one of these
wavet = maketable("random", 100, "even", min=-1, max=1, seed=1)
wavet = maketable("wave", 5000, "saw")
wavet = maketable("wave", 1000, "sine")

freqB1 = makeconnection("mouse", "x", min=100, max=1000, default=149.0,
				lag=70, "freqB1")
freqB2 = makeconnection("mouse", "y", min=100, max=1000, default=149.0,
				lag=70, "freqB2")

WAVY(start=0, dur, amp * env, freqA, freqB1, 0, wavet, 0, expr, pan=.6)

WAVY(start=0, dur, amp * env, freqA * 1.01, freqB2, 0, wavet, 0, expr, pan=.4)

