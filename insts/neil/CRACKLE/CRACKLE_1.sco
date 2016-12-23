rtsetparams(44100, 2)
load("./libCRACKLE.so")

chaos = maketable("line", "nonorm", 1000, 0,0, 1,1)
dur = 10
amp = 10000
CRACKLE(0, dur, amp, chaos, pan=0.9)
CRACKLE(0, dur, amp, chaos, pan=0.1)
