rtsetparams(44100, 2)
load("JDELAY")

rtinput("../../../snd/input.wav")

outskip = 0
inskip = 0
indur = DUR()
amp = 1.0
deltime = 1/cpspch(7.02)
feedback = .990
ringdur = 1
percent_wet = 0.5
prefadersend = 0  // if 0, sound stops abruptly; else env spans indur + ringdur

env = maketable("line", 1000, 0,0, 1,1, 6,1, 10,0)

cutoff = 4000
JDELAY(outskip, inskip, indur, amp * env, deltime, feedback, ringdur,
       cutoff, percent_wet, inchan=0, pan=0.5, prefadersend)
