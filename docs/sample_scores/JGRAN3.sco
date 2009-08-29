rtsetparams(44100, 2)
load("JGRAN")
load("REVERBIT")
bus_config("JGRAN", "aux 0-1 out")
bus_config("REVERBIT", "aux 0-1 in",  "out 0-1")

masteramp = 1.0

dur = 16

// overall amplitude envelope
env = maketable("line", 1000, 0,0, 1,1, 2,1, 4,0)

// grain envelope
genv = maketable("window", 10000, "hanning")

// grain waveform
gwave = maketable("wave", 10000, "sine")

// modulation frequency multiplier -- slightly increasing
mfreqmult = maketable("line", "nonorm", 1000, 0,2, 1,2.1)

// index of modulation envelope (per grain) -- increasing
modindex = maketable("line", "nonorm", 1000, 0,0, 1,5)

// grain frequency -- constant minimum, increasing maximum
minfreq = 500
maxfreq = maketable("line", "nonorm", 1000, 0,500, 1,550)

// grain speed -- decreasing minimum, constant maximum
minspeed = maketable("line", "nonorm", 1000, 0,100, 1,10)
maxspeed = 100

// grain intensity (decibels above 0)
mindb = 65
maxdb = 80

// grain density - slightly increasing
density = maketable("line", "nonorm", 1000, 0,1, 1,1, 2,.8)

// grain stereo location -- image centered in middle
pan = 0.5

// grain stereo location randomization -- increasingly randomized
panrand = maketable("line", "nonorm", 1000, 0,0, 1,1)


JGRAN(start=0, dur, 0.5 * env, seed=1, type=1, ranphase=1,
   genv, gwave, mfreqmult, modindex, minfreq, maxfreq, minspeed, maxspeed,
   mindb, maxdb, density, pan, panrand)

gwave = maketable("wave", 10000, 1, .5, .3, .2, .1)
minfreq = maketable("line", "nonorm", 1000, 0,900, 1,840)
maxfreq = maketable("line", "nonorm", 1000, 0,900, 1,1000)
panrand = maketable("line", "nonorm", 1000, 0,1, 1,0)

JGRAN(start=0, dur, env, seed=2, type=0, ranphase=1,
   genv, gwave, mfreqmult, modindex, minfreq, maxfreq, minspeed, maxspeed,
   mindb, maxdb, density, pan, panrand)

// --------------------------------------------------------------- reverb ---
revtime = 1.0
revpct = .8
rtchandel = .03
cf = 2000

REVERBIT(st=0, insk=0, dur, masteramp, revtime, revpct, rtchandel, cf)

