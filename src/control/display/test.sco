rtsetparams(44100, 2, 256)
load("WAVETABLE")

dur = 120

amp = makeconnection("mouse", "y", min=0, max=90, dflt=min, lag=50,
			"amp", "dB", 2)
amp = makeconverter(amp, "ampdb")
amp = makedisplay(amp, "amp")

freq = makerandom("linear", rfreq=3, min=200, max=2000)
freq = makedisplay(freq, "freq", "Hz", 0)

pan = makeconnection("mouse", "x", min=1, max=0, dflt=.5, lag=50, "pan")
pan = makedisplay(pan, "pan")

wave = maketable("wave", 2000, 1)

WAVETABLE(0, dur, amp, freq, pan, wave)

