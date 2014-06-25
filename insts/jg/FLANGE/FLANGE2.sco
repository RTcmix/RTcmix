rtsetparams(44100, 2)
load("FLANGE")

rtinput("a_stereo.snd")

outskip = 0
inskip = 0
dur = DUR()
amp = 0.5

resonance = 0.10
lowpitch = 8.00
moddepth = 80
modspeed = 0.5
wetdrymix = 0.5
flangetype = 0
rightchandelay = 0.08

gensize = 20000

makegen(2,10,gensize, 1)

maxdelay = 1.0 / cpspch(lowpitch)
FLANGE(outskip, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan=0, spread=1)

makegen(2,9,gensize, 1,1,45)    /* out of phase with left chan sine */

outskip = outskip + rightchandelay
maxdelay = maxdelay * 1.0001
FLANGE(outskip, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan=1, spread=0)
