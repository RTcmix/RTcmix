rtsetparams(44100, 2)
load("./libHENON.so")

x = maketable("line", 1000, 0,0, 2,1)
dur = 10
amp = 10000
HENON(0, dur, amp, a=1.10, b=0.3, x, y=0, cr=1000, pan=0)
HENON(0, dur, amp, a=1.11, b=0.3, x, y=0, cr=1000, pan=1)
