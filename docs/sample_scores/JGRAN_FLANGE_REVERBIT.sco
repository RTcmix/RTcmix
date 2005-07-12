// A synthy sound, made by fm granular synthesis, passes through a flanger
// and then a reverb.   -JGG, 6/17/00, rev for v4, 7/12/05

rtsetparams(44100, 2)
load("JGRAN")
load("FLANGE")
load("REVERBIT")

bus_config("JGRAN", "aux 0-1 out")
bus_config("FLANGE", "aux 0-1 in", "aux 2-3 out")
bus_config("REVERBIT", "aux 2-3 in",  "out 0-1")

totdur = 25
masteramp = 1.1

// ----------------------------------------------------------------- gran ---
genv = maketable("window", 10000, "hanning")
gwave = maketable("wave", 10000, "sine")
mfreqmult = 2.0
modindex = 5.0
minfreq = 300
maxfreq = 300
minspeed = 20
maxspeed = 40
mindb = 65
maxdb = 80
density = 1
pan = 0.5
panrand = 1

JGRAN(start=0, totdur, amp=1, seed=1, type=1, ranphase=0,
   genv, gwave, mfreqmult, modindex, minfreq, maxfreq, minspeed, maxspeed,
   mindb, maxdb, density, pan, panrand)

// --------------------------------------------------------------- flange ---
resonance = 0.6
maxdelay = 0.015
moddepth = 100
modspeed = 0.2
wetdrymix = 0.5
flangetype = "IIR"

waveform = maketable("wave", 20000, "sine")

FLANGE(st=0, insk=0, totdur, amp=1, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan=0, pan=1, ringdur=0, waveform)
maxdelay += 0.02
FLANGE(st=0, insk=0, totdur, amp=1, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan=1, pan=0, ringdur=0, waveform)

// --------------------------------------------------------------- reverb ---
revtime = 1.0
revpct = 0.5
rtchandel = 0.1
cf = 1000

env = maketable("line", 1000, 0,0, 1,1, 2,1, 3,0)

REVERBIT(st=0, insk=0, totdur, masteramp * env, revtime, revpct, rtchandel, cf)

