rtsetparams(44100, 2)
load("./libMCLAR.so")

amp = maketable("line", 1000, 0,0, 1,1, 2,1, 3,0)
MCLAR(0, 3.5, amp*20000.0, 314.0, 0.2, 0.7, 0.5)

bamp = maketable("line", 1000, 0,1, 2,1, 3,0)
MCLAR(4, 3.5, 20000.0, 149.0, 0.2, 0.7, 0.5, 0.5, bamp)
