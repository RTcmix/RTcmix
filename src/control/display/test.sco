rtsetparams(44100, 2, 256)
load("WAVETABLE")

dur = 120

amp = makeconnection("mouse", "y", min=0, max=90, dflt=min, lag=50,
			"amp", "dB", 2)
amp = makeconverter(amp, "ampdb")
amp = makedisplay(amp, "amp")


freq = makerandom("linear", rfreq=3, min=200, max=2000)
freq = makedisplay(freq, "freq", "Hz", 0)

pan = 0.5

wave = maketable("wave", 2000, 1)

WAVETABLE(0, dur, amp, freq, pan, wave)

