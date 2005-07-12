set_option("record = on")  // must do this before rtsetparams
rtsetparams(44100, 2)
load("PANECHO")
rtinput("AUDIO")

start = 0
inskip = 0
dur = 14
amp = 1.0
delayL = 5.14
delayR = 1.14
feedback = 0.7
ringdur = 9.5

PANECHO(start, inskip, dur, amp, delayL, delayR, feedback, ringdur)

start = 10
dur = 7
delayL = 1.14
delayR = 0.14
feedback = 0.7
ringdur = 3.5

PANECHO(start, inskip, dur, amp, delayL, delayR, feedback, ringdur)

