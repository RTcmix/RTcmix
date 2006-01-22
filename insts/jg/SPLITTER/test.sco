rtsetparams(44100, 4)
load("WAVETABLE")
load("SPLITTER")

dur = 10
amp = 10000
freq = 440
pan = 0	// ignored
wavet = maketable("wave", 1000, "sine")
bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, amp, freq, pan, wavet)
bus_config("WAVETABLE", "aux 1 out")
freq = 770
WAVETABLE(0, dur, amp, freq, pan, wavet)

bus_config("SPLITTER", "aux 0-1 in", "out 0", "out 2-3")
inchan = 0
SPLITTER(0, 0, dur, 1, inchan)

