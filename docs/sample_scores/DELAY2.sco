rtsetparams(44100, 2)
load("DELAY")

rtinput("../../snd/input.wav")
inchan = 0
dur = DUR()

amp = maketable("line", 1000, 0,0, 0.5,1, 3.5,1, 7,0) * 0.5
DELAY(0, 0, dur, amp, deltime=.14, feedback=.7, ringdur=3.5, inchan, pan=.1)

amp = maketable("line", 1000, 0,0, 0.1,1, 1.5,0.21, 3.5,1, 7,0)
DELAY(3.5, 0, dur, amp, deltime=1.4, feedback=.3, ringdur=5, inchan, pan=.9)

