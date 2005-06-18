rtsetparams(44100, 2)
load("JGRAN")
bus_config("JGRAN", "out 0-1")

dur = 10
amp = .5

// overall amplitude envelope
env = maketable("line", 1000, 0,0, 6,1, 9,1, 10,0)

// grain envelope
genv = maketable("window", 10000, "hanning")

// grain waveform
gwave = maketable("wave", 10000, 1, .1, .05)

// modulation frequency multiplier
mfreqmult = maketable("random", "nonorm", 1000, "high", min=0, max=1, seed=1)

// index of modulation envelope (per grain)
modindex = maketable("line", "nonorm", 1000, 0,0, 1,6)

// grain frequency
minfreq = maxfreq = 500

// grain speed
minspeed = 20
maxspeed = 90

// grain intensity (decibels above 0)
mindb = 80
maxdb = 80

density = 1

// grain stereo location
pan = 0.5

// grain stereo location randomization
panrand = maketable("line", "nonorm", 1000, 0,0, 1,1)

JGRAN(start=0, dur, amp * env, seed=1, type=1, ranphase=1,
   genv, gwave, mfreqmult, modindex, minfreq, maxfreq, minspeed, maxspeed,
   mindb, maxdb, density, pan, panrand)


// a second grain stream, with some different params
env = maketable("line", 1000, 0,0, 1,1, 4,1, 10,0)
minfreq = 1000
maxfreq = 1100
panrand = maketable("line", "nonorm", 1000, 0,1, 1,0)
amp = 2
JGRAN(start=0, dur, amp * env, seed=2, type=0, ranphase=1,
   genv, gwave, mfreqmult, modindex, minfreq, maxfreq, minspeed, maxspeed,
   mindb, maxdb, density, pan, panrand)


