rtsetparams(44100, 2)
load("FMINST")

dur = 7

amp = maketable("line", 1000, 0, 0, 3.5,1, 7,0) * 20000

carfreq = cpspch(8.00)
modfreq = 179
minindex = 0
maxindex = 10
pan = 0.1

wavetable = maketable("wave", 1000, "sine")
guide = maketable("line", "nonorm", 1000, 0, 0, 5,1, 7, 0)

FMINST(0, dur, amp, carfreq, modfreq, minindex, maxindex, pan,
	wavetable, guide)

start = dur / 2
carfreq = cpspch(8.07)
pan = 0.9
guide = maketable("line", "nonorm", 1000, 0,1, 7,0)

FMINST(start, dur, amp, carfreq, modfreq, minindex, maxindex, pan,
	wavetable, guide)

