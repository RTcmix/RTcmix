rtsetparams(44100, 2)
load("WAVETABLE")

dur = 15
amp = 8000
env = maketable("curve", 1000, 0,0,1, 1,1,0, 3,1,-1, 4,0)

freq = 30
wavet = maketable("random", 4000, "even", -1, 1, seed=1)
WAVETABLE(0, dur, amp * env, freq, pan=0, wavet)

wavet = maketable("random", 3000, "even", -1, 1, seed=2)
WAVETABLE(0, dur, amp * env, freq, pan=1, wavet)

