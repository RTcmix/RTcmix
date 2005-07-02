rtsetparams(44100, 2)
load("COMBIT")

rtinput("../../snd/input.wav")
inchan = 0
inskip = 0
dur = DUR()

amp = 1.5
env = maketable("line", 1000, 0,0, 2,1, 3.5,1)

pitch = 6.07
rvbtime = 5.0

COMBIT(start=0, inskip, dur, amp * env, cpspch(pitch), rvbtime, 0, pan=0.5)

