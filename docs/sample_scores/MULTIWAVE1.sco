rtsetparams(44100, 2)
load("MULTIWAVE")

dur = 60
amp = 10000

wave = maketable("wave", 2000, 1)
line = maketable("line", 1000, 0,0, 1,1, 9,1, 10,0)

rfreq = 1
freq1 = makerandom("linear", rfreq, min=200, max=1000, seed=1)
freq1 = makefilter(freq1, "smooth", 90)

lag = 60
amp1 = makeconnection("mouse", "Y", 0, 1, 0, lag, "amp")
pan1 = makeconnection("mouse", "X", 1, 0, .5, lag, "pan")

MULTIWAVE(0, dur, amp * line, wave,
   freq1, amp1, phase1=0, pan1,
   freq1 * 1.05, amp1, phase1, pan1 * .7)


// JGG, 3/10/05

