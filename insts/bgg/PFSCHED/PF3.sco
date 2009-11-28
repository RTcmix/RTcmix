rtsetparams(44100, 1)
reset(10000)
load("PFSCHED")
load("WAVETABLE")

delayed_envelope = maketable("line", 1000, 0,0.0,  1,1.0)
value = makeconnection("pfbus", 1, 0.0)
PFSCHED(0.5, 0.01, 1, delayed_envelope)

delayed_envelope2 = maketable("line", 1000, 0,1.0,  1,0.0)
value2 = makeconnection("pfbus", 2, 1.0)
PFSCHED(3.5, 3.5, 2, delayed_envelope2)

delayed_envelope3 = maketable("line", 1000, 0,1,  1,0, 2, 0.5, 3,1, 4,0, 5,1)
value3 = makeconnection("pfbus", 3, 1.0)
PFSCHED(1.5, 5.5, 3, delayed_envelope3)

wave = maketable("wave", 1000, "sine")

WAVETABLE(0, 7.0, 20000*value*value2*value3, 8.00, 0.5, wave)

