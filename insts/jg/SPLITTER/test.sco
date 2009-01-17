rtsetparams(44100, 2)
load("WAVETABLE")
load("SPLITTER")

dur = 10
amp = 5000
freq = 440
pan = 0	// ignored
wavet = maketable("wave", 1000, "sine")
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, amp, freq, pan, wavet)
bus_config("WAVETABLE", "aux 1 out")
freq = 770
WAVETABLE(0, dur, amp, freq, pan, wavet)

bus_config("SPLITTER", "aux 0 in", "aux 10 out", "aux 12 out")
inchan = 0
SPLITTER(0, 0, dur, 1, inchan, 1, 1)
bus_config("SPLITTER", "aux 1 in", "aux 11 out", "aux 13 out")
SPLITTER(0, 0, dur, 1, inchan, 1, 1)

//bus_config("MIX", "aux 10-11 in", "out 0-1")
bus_config("MIX", "aux 12-13 in", "out 0-1")
MIX(0, 0, dur, 1, 0, 1)
