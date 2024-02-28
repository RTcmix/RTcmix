rtsetparams(44100, 2)
load("TRANS")

rtinput("../../snd/input.wav")
dur = DUR()

amp = 1.8
env = maketable("line", 1000, 0,0, 1,1, 9,1, 10,0)

ratio = 0.25	/* 1/4 as fast */

pan = maketable("line", "nonorm", 1000, 0,0.5, 1,0.2, 3,1)

RTRANS3(start=0, inskip=0, dur/ratio, amp * env, ratio, 0, pan)

