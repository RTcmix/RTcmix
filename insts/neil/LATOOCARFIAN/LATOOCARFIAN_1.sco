rtsetparams(44100, 2)
load("./libLATOOCARFIAN.so")

a = maketable("line", "nonorm", 1000, 0,2.855, 1,3)
b = maketable("line", "nonorm", 1000, 0,2.1, 1,2.9)
LATOOCARFIAN(0, dur=10, amp=10000, a, b)
