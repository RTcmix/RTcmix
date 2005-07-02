rtsetparams(44100, 2)
load("DELAY")

rtinput("../../snd/loocher.aiff")
inchan = 0
inskip = 0
dur = DUR()

env = maketable("line", 1000, 0,0, 0.5,1, 3.5,1, 7,0)
deltime = 0.14
feedback = 0.7
ringdur = 3.5
pan = 0.2

DELAY(start=0, inskip, dur, env * 0.7, deltime, feedback, ringdur, inchan, pan)

start = 3.5
env = maketable("line", 1000, 0,0, 0.1,1, 1.5,0.21, 3.5,1, 7,0)
deltime = 1.4
feedback = 0.3
ringdur = 5
pan = 0.8

DELAY(start, inskip, dur, env * 1, deltime, feedback, ringdur, inchan, pan)

