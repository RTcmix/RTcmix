// FM with a random index guide function -- different for each channel.  FM
// feeds into resonators.  Smoothness of random changes controlled with mouse.
// -JG, 6/17/00; v4 rev, 7/4/05

rtsetparams(44100, 2)
load("FMINST")
load("JDELAY")

totdur = 30

// ------------------------------------------------------------------- fm ---
amp = 17000
car = 200
mod = car * (7 / 5)

imin = 0
imax = 15

// index guide determined by random stream
speed = 30         // in Hz
seed = 1331
distribution = "low"
guide = makerandom(distribution, speed, min=0, max=1, seed)
// lag controls the smoothness of transitions between random index values
lag = makeconnection("mouse", "x", min=0, max=100, dflt=50, 50, "leftlag [x]")
guide = makefilter(guide, "smooth", lag)

wavet = maketable("wave", 1000, "sine")

bus_config("FMINST", "aux 0 out")
FMINST(st=0, totdur, amp, car, mod, imin, imax, pan=0, wavet, guide)

// make 2nd chan sound different by changing its index guide
speed = 26
seed = 9240
guide = makerandom(distribution, speed, min=0, max=1, seed)
lag = makeconnection("mouse", "y", min=0, max=100, dflt=50, 50, "rightlag [y]")
guide = makefilter(guide, "smooth", lag)

bus_config("FMINST", "aux 1 out")
FMINST(st=0, totdur, amp, car, mod, imin, imax, pan=0, wavet, guide)

// ------------------------------------------------------------ resonator ---
pitch = 5.02
regen = 0.985
wetdry = 0.18
cutoff = 0
ringdur = 8.0

amp = maketable("line", 1000, 0,0, 1,1, 19,1, 20,0) * 0.5

bus_config("JDELAY", "aux 0 in", "out 0")
deltime = 1 / cpspch(pitch)
JDELAY(st=0, insk=0, totdur, amp, deltime, regen, ringdur, cutoff, wetdry)

bus_config("JDELAY", "aux 1 in", "out 1")
deltime = 1 / cpspch(pitch + .002)
JDELAY(st=0, insk=0, totdur, amp, deltime, regen, ringdur, cutoff, wetdry)

/* Note that it's also possible to have one bus_config for both JDELAY's:
      bus_config("JDELAY", "aux 0-1 in", "out 0-1")
   and then using the optional inchan and pctleft pfields to select the
   in and out chans for each note. But this requires a bit more CPU.
*/

