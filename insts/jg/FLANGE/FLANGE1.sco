rtsetparams(44100, 2)
load("FLANGE")

rtinput("any.snd")

outskip = 0
inskip = 0
dur = DUR()
amp = 1.0
inchan = 0
spread = 0.5

resonance = 0.06
lowpitch = 7.00
moddepth = 70
modspeed = 2.5
wetdrymix = 0.5
flangetype = 0

gensize = 2000
makegen(2,10,gensize, 1)

maxdelay = 1.0 / cpspch(lowpitch)

FLANGE(outskip, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan, spread)
