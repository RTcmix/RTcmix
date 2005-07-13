rtsetparams(44100, 2)
load("EQ")

rtinput("../../snd/loocher.aiff")
inskip = 0
dur = DUR()

start = 2
amp = 1
bypass = 0

type = "lowpass"
type = "highpass"
type = "lowshelf"
type = "highshelf"
type = "peaknotch"

pan = maketable("line", 100, 0,1, 1,0)
freq = makeconnection("mouse", "x", min=200, max=2000, dflt=min, lag=50,
			"freq", "Hz", 1)
Q = 1.0
gain = makeconnection("mouse", "y", min=-30, max=20, dflt=0, lag=50,
			"gain", "dB", 1)

EQ(start, inskip, dur, amp, type, inchan=0, pan, bypass, freq, Q, gain)

