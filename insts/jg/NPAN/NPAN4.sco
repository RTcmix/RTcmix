rtsetparams(44100, 8, 512)
load("WAVETABLE")
load("NPAN")

// 8 speakers arranged in a rectangle, as follows...
//
//    1  7  2
//    3  x  4        x = listener
//    5  8  6

sin45 = 0.70710678

NPANspeakers("xy",
   -1,     1,     // front left (1)
    1,     1,     // front right (2)
   -sin45, 0,     // side left (3)
    sin45, 0,     // side right (4)
   -1,    -1,     // rear left (5)
    1,    -1,     // rear right rear (6)
    0, sin45,     // front center (7)
    0, -sin45)    // rear center (8)

dur = 60
amp = 10000
freq = 440

wave = maketable("wave", 2000, 1)

bus_config("WAVETABLE", "aux 0 out")
line = maketable("line", 1000, 0,0, 1,1, 49,1, 50,0)
WAVETABLE(0, dur, amp * line, freq, 0, wave)

//----------------------------------------------------------------
bus_config("NPAN", "aux 0 in", "out 0-7")

lag = 70
x = makeconnection("mouse", "X", -1, 1, 0, lag, "X")
y = makeconnection("mouse", "Y", -1, 1, 1, lag, "Y")

NPAN(0, 0, dur, 1, "xy", x, y)

