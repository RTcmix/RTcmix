rtsetparams(44100, 1)
load("PFSCHED")
load("WAVETABLE")

sent_envelope = maketable("line", 100, 0,0.0,  1,1.0)
value = makeconnection("pfbus", 1, 0.0)

wave = maketable("wave", 1000, "sine")

PFSCHED(2.5, 2.1, 1, sent_envelope, 1)
WAVETABLE(0, 7.0, 20000*value, 8.00, 0.5, wave)

ampenv = maketable("line", 1000, 0,0, 1,1, 5,0.5, 6,1, 7,0)
WAVETABLE(0, 7.0, 20000*ampenv, 8.09, 0.5, wave)
