rtsetparams(44100, 1)
load("PFSCHED")
load("WAVETABLE")

sent_envelope = maketable("line", 1000, 0,0.0,  1,1.0)
value = makeconnection("pfbus", 1, 0.0)

wave = maketable("wave", 1000, "sine")

PFSCHED(0.5, 0.1, 1, sent_envelope)
WAVETABLE(0, 77.0, 10000*value, 8.00, 0.5, wave)

PFSCHED(0, 2.1, 1, sent_envelope)
WAVETABLE(3, 77.0, 10000*value, 8.05, 0.5, wave)

