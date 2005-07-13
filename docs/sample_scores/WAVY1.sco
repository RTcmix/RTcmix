rtsetparams(44100, 2, 256)
load("WAVY")

// set to true (or 1) to let you muck around (blindly) with the waveform
wavedraw = false

dur = 60
amp = 10000
env = maketable("line", 1000, 0,0, .1,1, dur-.1,1, dur,0)
pitch = 6.00

low = 0.01
high = 0.99
lfreq = 0.1
phase_offset1 = makeLFO("tri", lfreq, min=low, max=high)

// try one of these
expr = "(a ^ 5) * .5"
expr = "a - b"
expr = "if (abs(a) < .5, a ^ .1, a * (b - 1))"

// try one of these
wavet = maketable("random", 100, "even", min=-1, max=1, seed=1)
wavet = maketable("wave", 5000, "saw")

if (wavedraw) {
	index = makeconnection("mouse", "x", min=0, max=1, dflt=0, lag=80, "index")
	value = makeconnection("mouse", "y", min=-1, max=1, dflt=0, lag=80, "value")
	wavet = modtable(wavet, "draw", index, value, .1)
}

WAVY(start=0, dur, amp * env, pitch, 0, phase_offset1, wavet, 0, expr, pan=.6)

// need a separate LFO instance for this
phase_offset2 = makeLFO("tri", lfreq + 0.02, min=low, max=high)

pitch += 0.001
WAVY(start=0, dur, amp * env, pitch, 0, phase_offset2, wavet, 0, expr, pan=.4)

