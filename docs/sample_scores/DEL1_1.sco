rtsetparams(44100, 2)
load("DEL1")

rtinput("../../snd/huhh.wav")
dur = DUR()

env = maketable("line", 1000, 0,0, 0.5,1, 3.5,1, 7,0)
rtchandel = 0.001
rtchanamp = 1
DEL1(0, 0, dur, env, rtchandel, rtchanamp)

env = maketable("line", 1000, 0,0, 0.1,1, 1.5,0.21, 3.5,1, 7,0)
rtchandel = 0.14
rtchanamp = 1
DEL1(dur + .5, 0, dur, env, rtchandel, rtchanamp)

