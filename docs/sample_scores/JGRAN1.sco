rtsetparams(44100, 2)
load("JGRAN")
bus_config("JGRAN", "out 0-1")

dur = 16
amp = 1

// overall amplitude envelope
env = maketable("line", 1000, 0,0, 1,1, 2,1, 4,0)

// grain envelope
genv = maketable("window", 1000, "hanning")

// grain waveform
gwave = maketable("wave", 10000, "sine")

// modulation frequency multiplier
mfreqmult = maketable("line", "nonorm", 1000, 0,2, 1,2.1) // slight increase

// index of modulation envelope (per grain)
modindex = maketable("line", "nonorm", 1000, 0,0, 1,5) // increasing index

// grain frequency
minfreq = 500
maxfreq = maketable("line", "nonorm", 1000, 0,500, 1,550) // increasing maximum

// grain speed
minspeed = maketable("line", "nonorm", 1000, 0,100, 1,10) // decreasing minimum
maxspeed = 100

// grain intensity (decibels above 0)
mindb = 80
maxdb = 80

// grain density
density = maketable("line", "nonorm", 1000, 0,1, 1,1, 2,.8)  // slight decrease

// grain stereo location -- image centered in middle
pan = 0.5

// grain stereo location randomization
panrand = maketable("line", "nonorm", 1000, 0,0, 1,1) // increasingly randomized


JGRAN(start=0, dur, amp * env, seed=1, type=1, ranphase=1,
   genv, gwave, mfreqmult, modindex, minfreq, maxfreq, minspeed, maxspeed,
   mindb, maxdb, density, pan, panrand)

