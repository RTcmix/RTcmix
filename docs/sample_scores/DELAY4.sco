rtsetparams(44100, 2)
load("DELAY")

rtinput("../../snd/nucular.wav")
inchan = 0
inskip = 0
dur = DUR()

amp = 1.5
env = maketable("line", 1000, 0,1, dur*.8,1, dur,0)

del1 = 0.01
del2 = 0.2
deltime = maketable("curve", "nonorm", 1000, 0,del1,3, dur/2,del2,0, dur,del1)

delamp = 0.8
ringdur = 1.0

pan = maketable("curve", 1000, 0,0,-2, dur,1)

control_rate(20000)
DELAY(start=0, inskip, dur, amp * env, deltime, delamp, ringdur, inchan, pan)

