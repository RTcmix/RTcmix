rtsetparams(44100, 2)
load("COMBIT")

rtinput("../../snd/huhh.wav")
dur = DUR()

amp = 0.8
env = maketable("line", 1000, 0,0, 1,1, 7,1, 10,0)

freq = maketable("random", "nonorm", dur * 8, "cauchy", 1, 50, 180)
rvbtime = maketable("line", "nonorm", 1000, 0,1, 1,8)
pctleft = maketable("wave", "nonorm", 1000, .5) + 0.5
ringdur = 0.5
reset(10000)

COMBIT(0, 0, dur, amp * env, freq, rvbtime, 0, pctleft, ringdur)

