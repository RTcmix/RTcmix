rtsetparams(44100, 4)
load("WAVETABLE")
load("QPAN")

dur = 60
amp = 10000
freq = 440

wave = maketable("wave", 2000, 1)
line = maketable("line", 1000, 0,0, 1,1, 19,1, 20,0)

bus_config("WAVETABLE", "aux 0 out")
WAVETABLE(0, dur, amp * line, freq, 0, wave)

//----------------------------------------------------------------
bus_config("QPAN", "aux 0 in", "out 0-3")

lag = 70
srcX = makeconnection("mouse", "X", -1, 1, 0, lag, "X")
srcY = makeconnection("mouse", "Y", -1, 1, 1, lag, "Y")

QPAN(0, 0, dur, 1, srcX, srcY)

