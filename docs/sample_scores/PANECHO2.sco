rtsetparams(44100, 2)
load("PANECHO")

rtinput("../../snd/input.wav")

start = 0
inskip = 0
dur = DUR()
amp = 0.9
env = maketable("line", "nonorm", 1000, 0,0, 0.5,1, 3.5,1, 7,0)
delayL = 0.14
delayR = 0.14
feedback = 0.7
ringdur = 3.5

PANECHO(start, inskip, dur, amp * env, delayL, delayR, feedback, ringdur)

start = 4.9
amp = 0.9
env = maketable("line", "nonorm", 1000, 0,0, 1.5,1, 3.5,1, 7,0)
delayL = 0.514
delayR = 0.05
feedback = 0.95
ringdur = 24.0

PANECHO(start, inskip, dur, amp * env, delayL, delayR, feedback, ringdur)

