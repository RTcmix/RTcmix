rtsetparams(44100, 2)
load("FLANGE")

rtinput("../../../snd/input.wav")

outskip = 0
inskip = 0
dur = DUR()
amp = 1.0
inchan = 0
pan = 0.5
ringdur = 0

resonance = 0.06
lowpitch = 7.00
moddepth = 70
modspeed = 2.5
wetdrymix = 0.5
flangetype = "IIR"

modwavet = maketable("wave", 2000, "sine")

maxdelay = 1.0 / cpspch(lowpitch)

FLANGE(outskip, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan, pan, ringdur, modwavet)
