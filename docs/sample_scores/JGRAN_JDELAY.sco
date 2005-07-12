// -JGG, 5/28/00, rev for v4, 7/12/05

rtsetparams(44100, 2)
load("JGRAN")
load("JDELAY")

bus_config("JGRAN", "aux 0-1 out")
bus_config("JDELAY", "aux 0-1 in", "out0-1")

dur = 16
masteramp = 1.4

//----------------------------------------------------------------------------
// overall amplitude envelope
env = maketable("line", 1000, 0,0, 1,1, 2,1, 4,0)

// grain envelope
genv = maketable("window", 10000, "hanning")

// grain waveform
gwave = maketable("wave", 10000, "sine")

// modulation frequency multiplier
mfreqmult = maketable("line", "nonorm", 1000, 0,2, 1,2.1) // slight increase

// index of modulation envelope (per grain)
modindex = maketable("line", "nonorm", 1000, 0,0, 1,8) // increasing index

// grain frequency
minfreq = 500
maxfreq = maketable("line", "nonorm", 1000, 0,450, 1,550) // increasing maximum

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


JGRAN(start=0, dur, env, seed=1, type=1, ranphase=1,
   genv, gwave, mfreqmult, modindex, minfreq, maxfreq, minspeed, maxspeed,
   mindb, maxdb, density, pan, panrand)

//----------------------------------------------------------------------------
deltime1 = 0.37
deltime2 = 0.38
regen = 0.88
wetdry = 0.75
ringdur = 14.0
cutoff = 0

JDELAY(st=0, insk=0, dur, masteramp, deltime1, regen, ringdur, cutoff, wetdry,
       inchan=0, pan=1)
JDELAY(st=0, insk=0, dur, masteramp, deltime2, regen, ringdur, cutoff, wetdry,
       inchan=1, pan=0)

