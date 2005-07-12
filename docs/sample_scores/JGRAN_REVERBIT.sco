// JGG, 5/28/00, rev for v4, 7/12/05

rtsetparams(44100, 2)
load("JGRAN")
load("REVERBIT")
bus_config("JGRAN", "aux 0-1 out")
bus_config("REVERBIT", "aux 0-1 in",  "out 0-1")

masteramp = 1.0

dur = 20

env = maketable("line", 1000, 0,0, 1,1, 2,1, 4,0)

genv = maketable("window", 10000, "hanning")
gwave = maketable("wave", 10000, "sine")

mfreqmult = maketable("line", "nonorm", 1000, 0,2, 1,2.2)
modindex = maketable("line", "nonorm", 1000, 0,0, 1,9)

minfreq = maketable("line", "nonorm", 1000, 0,200, 1,100)
maxfreq = maketable("line", "nonorm", 1000, 0,200, 1,550)

minspeed = maketable("line", "nonorm", 1000, 0,100, 1,10)
maxspeed = 100

mindb = 65
maxdb = 80

density = maketable("line", "nonorm", 1000, 0,1, 1,1, 2,.8)

pan = 0.5	// image centered in middle

// grain stereo location randomization -- decreasingly randomized
panrand = maketable("line", "nonorm", 1000, 0,1, 1,0)

JGRAN(start=0, dur, env, seed=0, type=1, ranphase=1,
   genv, gwave, mfreqmult, modindex, minfreq, maxfreq, minspeed, maxspeed,
   mindb, maxdb, density, pan, panrand)

env = maketable("line", 1000, 0,0, 1,.1, 3,1, 4,0)
gwave = maketable("wave", 10000, 1, .5, .3, .2, .1)
minfreq = 2000
maxfreq = 2100
panrand = maketable("line", "nonorm", 1000, 0,0, 1,1)

JGRAN(start=2, dur-start, env, seed=1, type=0, ranphase=1,
   genv, gwave, mfreqmult, modindex, minfreq, maxfreq, minspeed, maxspeed,
   mindb, maxdb, density, pan, panrand)

// --------------------------------------------------------------- reverb ---
revtime = 3.0
revpct = maketable("line", "nonorm", 1000, 0,0.8, 1,0.8, 2,0.1)
rtchandel = .05
cf = 800

REVERBIT(st=0, insk=0, dur, masteramp, revtime, revpct, rtchandel, cf)

