rtsetparams(44100, 2)
load("WAVETABLE")
load("FLANGE")

bus_config("WAVETABLE", "aux 0-1 out")
bus_config("FLANGE", "aux 0-1 in", "out 0-1")

makegen(2, 10, 10000, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1)

start = 0
dur = 20
amp = 12000

reset(5000)
setline(0,0, 1,1, 19,1, 20,0)

WAVETABLE(start, dur, amp, 6.00, pctleft=1)
WAVETABLE(start, dur, amp, 7.00, pctleft=.5)
WAVETABLE(start, dur, amp, 7.07, pctleft=0)


/*----------------------------------------------------------------------------*/
start = 0
inskip = 0      /* must be zero when reading from aux bus! */

amp = 1.0

resonance = 0.2
lowpitch = 5.00
moddepth = 90
modspeed = 0.05
wetdrymix = 0.5
flangetype = 0

gensize = 100000
makegen(2,10,gensize, 1)

maxdelay = 1.0 / cpspch(lowpitch)

setline(0,1,1,1)

FLANGE(start, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan=0, pctleft=1)

lowpitch = lowpitch + .07
maxdelay = 1.0 / cpspch(lowpitch)

makegen(2,9,gensize, 1,1,-180)

FLANGE(start, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan=1, pctleft=0)

