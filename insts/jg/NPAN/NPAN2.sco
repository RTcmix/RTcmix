rtsetparams(44100, 4)
load("WAVETABLE")
load("NPAN")

dur = 10
amp = 10000
freq = 440

wave = maketable("wave", 2000, 1)

bus_config("WAVETABLE", "aux 0 out")
line = maketable("line", 1000, 0,0, 1,1, 19,1, 20,0)
WAVETABLE(0, dur, amp * line, freq, 0, wave)

//----------------------------------------------------------------
bus_config("NPAN", "aux 0 in", "out 0-3")

NPANspeakers("polar",
    45, 1,     // left front
   -45, 1,     // right front
   135, 1,     // left rear
  -135, 1)     // right rear

dist = 1

// counter-clockwise from left front
//angle = maketable("line", "nonorm", 1000, 0,45, 1,405)

// 3 counter-clockwise trips around circle
//trips = 3
//angle = maketable("linebrk", "nonorm", 1000, 45, 1000, 405 * trips)

// instantaneous change (w/ clicks)
//angle = maketable("literal", "nonorm", "nointerp", 0, 45, -45, 135, -135, -135)


NPAN(0, 0, dur, 1, "polar", angle, dist)

