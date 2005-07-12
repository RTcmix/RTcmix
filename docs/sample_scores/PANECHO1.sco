rtsetparams(44100, 2)
load("PANECHO")

rtinput("../../snd/input.wav")

start = 0
inskip = 0
dur = DUR()
amp = maketable("line", "nonorm", 1000, 0,0, 0.5,1, 3.5,1, 7,0)
delayL = 0.14
delayR = 0.069
feedback = 0.7
ringdur = 3.5

PANECHO(start, inskip, dur, amp, delayL, delayR, feedback, ringdur)

start = 4.9
amp = maketable("line", "nonorm", 1000, 0,0, 0.1,1, 1.5,0.21, 3.5,1, 7,0)
delayL = 1.14
delayR = 0.5
feedback = 0.35
ringdur = 9.5

PANECHO(start, inskip, dur, amp, delayL, delayR, feedback, ringdur)

