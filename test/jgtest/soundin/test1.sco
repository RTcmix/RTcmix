rtsetparams(44100, 1)

openfile("/snd/Public_Sounds/jimi3dSoundDN.aiff")
inskip = 4.5
dur = 4.0
amp = 1.0
inchan = 0
pctleft = 0.2

soundin(outskip=0, inskip, dur, amp, inchan, pctleft=1)
xsoundin(outskip=0.05, inskip, dur, amp, inchan, pctleft=0)

