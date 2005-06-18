rtsetparams(44100, 2, 256)
load("JGRAN")
bus_config("JGRAN", "out 0-1")

dur = 60
amp = 1

// overall amplitude envelope
env = maketable("line", 1000, 0,0, 1,1, 2,1, 4,0)

// grain envelope
genv = maketable("window", 1000, "hanning")

// grain waveform
gwave = maketable("wave", 10000, "sine")

// modulation frequency multiplier
mfreqmult = 0

// index of modulation envelope (per grain)
modindex = 0

// grain frequency
minfreq = maxfreq = 500
//maxfreq = maketable("line", "nonorm", 1000, 0,500, 1,550)

// grain speed
//minspeed = maketable("line", "nonorm", 1000, 0,100, 1,10)
minspeed = maxspeed = makeconnection("mouse", "x", 1, 200, 1, 20, "speed")

// grain intensity (decibels above 0)
mindb = 80
maxdb = 80

// grain density
//density = maketable("line", "nonorm", 1000, 0,1, 1,1, 2,.8)
density = makeconnection("mouse", "y", 1, 100, 1, 20, "density")

// grain stereo location
pan = 0.5

// grain stereo location randomization
panrand = 0


JGRAN(start=0, dur, amp * env, seed=0, type=0, ranphase=1,
   genv, gwave, mfreqmult, modindex, minfreq, maxfreq, minspeed, maxspeed,
   mindb, maxdb, density, pan, panrand)

