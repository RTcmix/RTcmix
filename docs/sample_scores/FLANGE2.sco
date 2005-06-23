rtsetparams(44100, 2)
load("FLANGE")

rtinput("../../snd/nucular.wav")
inchan = 0

start = 0
inskip = 0
dur = DUR()
amp = 1.5

resonance = 0.10
lowpitch = 8.00
moddepth = 80
modspeed = 0.5
wetdrymix = 0.5
flangetype = "IIR"
rightchandelay = 0.08
ringdur = 0		// let inst figure it out

waveform = maketable("wave", 1000, "sine")

maxdelay = 1.0 / cpspch(lowpitch)
FLANGE(start, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan, pan=1, ringdur, waveform)

// 45 deg out of phase with left chan sine
waveform = maketable("wave3", 1000, 1, 1, 45)

start += rightchandelay
maxdelay *= 1.0001
FLANGE(start, inskip, dur, amp, resonance, maxdelay, moddepth, modspeed,
       wetdrymix, flangetype, inchan, pan=0, ringdur, waveform)
