rtsetparams(44100, 2)
load("COMBIT")

rtinput("../../snd/input.wav")
inchan = 0
inskip = 0
dur = DUR()

amp = 0.5
freq = cpspch(7.09)
reverbtime = 0.9

COMBIT(start=0, inskip, dur, amp, freq, reverbtime, inchan, pan=0)

freq = cpspch(7.07)
COMBIT(start=0.2, inskip, dur, amp, freq, reverbtime, inchan, pan=1)

