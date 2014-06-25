rtsetparams(44100, 1)
load("PFSCHED")
load("WAVETABLE")

delayed_envelope = maketable("line", 100, 0,0.0,  1,1.0)
value = makeconnection("pfbus", 1, 0.0)

wave = maketable("wave", 1000, "sine")

PFSCHED(2.5, 2.1, 1, delayed_envelope)
WAVETABLE(0, 7.0, 20000*value, 8.00, 0.5, wave)

