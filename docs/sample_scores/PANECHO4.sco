rtsetparams(44100, 2)
load("PANECHO")
rtinput("../../snd/nucular.wav")
dur = DUR()

amp = 1.1
env = maketable("line", 1000, 0,0, 1,1, 8,1, 9,0)

deltimeL = maketable("line", "nonorm", 1000, 0,0.2, 1,0.6, 3,0.01)
deltimeR = maketable("random", "nonorm", dur * 10, "even", 0.1, 0.3, 2)
feedback = maketable("line", "nonorm", 100, 0,.2, 1,1, 6,.5)
ringdur = 8

reset(20000)
PANECHO(0, 0, dur, amp * env, deltimeL, deltimeR, feedback, ringdur)

