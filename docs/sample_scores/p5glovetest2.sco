// See p5glovetest.sco for what this is about.

rtsetparams(44100, 2, 256)
load("FMINST")
load("DELAY")

dur = 180

// default port for the p5osc driver
set_option("osc_inport = 47110")

// up/down for amp
amp = makeconnection("osc", "/p5glove_data", index=9,
         inmin=-500, inmax=500, outmin=20, outmax=92, dflt=0, lag=70)
amp = makeconverter(amp, "ampdb")

// left/right for carrier
car = makeconnection("osc", "/p5glove_data", index=8,
         inmin=-500, inmax=500, outmin=5000, outmax=50, dflt=500, lag=0)
mod = car * (7 / 5)

// forward/backward for mod index guide
guide = makeconnection("osc", "/p5glove_data", index=10,
         inmin=-500, inmax=500, outmin=1, outmax=0, dflt=0, lag=70)

imin = 0
imax = 50
pan = makerandom("even", rfreq=0.5, .1, .9, seed=1)

wavet = maketable("wave", 5000, "sine")

bus_config("FMINST", "aux 0-1 out")
FMINST(0, dur, amp, car, mod, imin, imax, pan, wavet, guide)

bus_config("DELAY", "aux 0 in", "out 0")
DELAY(0, 0, dur, 1, deltime=.25, fb=.4, ringdur=2)
bus_config("DELAY", "aux 1 in", "out 1")
DELAY(0, 0, dur, 1, deltime=.43, fb=.4, ringdur=2)

