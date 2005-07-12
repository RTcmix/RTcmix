rtsetparams(44100, 2)
load("TRANS")

rtinput("../../snd/input.wav")
dur = DUR()

amp = 1.8
env = maketable("line", 1000, 0,0, 1,1, 9,1, 10,0)

transp = -2.0

pan = maketable("line", "nonorm", 1000, 0,0.5, 1,0.2, 3,1)

dur = translen(dur, transp)
TRANS(start=0, inskip=0, dur, amp * env, transp, 0, pan)

