rtsetparams(44100, 2)
load("FMINST")

dur = 7
amp = 30000
carfreq = cpspch(8.00)
modfreq = 179
minindex = 0
maxindex = 10

env = maketable("line", 1000, 0, 0, 3.5,1, 7,0)
wavetable = maketable("wave", 1000, "sine")
guide = maketable("line", "nonorm", 1000, 0,1, 7,0)

FMINST(0, dur, amp * env, carfreq, modfreq, minindex, maxindex, pan=0.5,
	wavetable, guide)

