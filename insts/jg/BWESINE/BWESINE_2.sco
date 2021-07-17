rtsetparams(44100, 2)
load("./libBWESINE.so")

dur = 10
maxamp = 8000
amp = maketable("line", "nonorm", 1000, 0,0, 1,maxamp, 9,maxamp, 10,0)
wavet = maketable("wave", 32767, "sine")
freq = maketable("line", "nonorm", 1000, 0,220, 1,220, 6,880, 7,880)
bw = maketable("curve", "nonorm", 1000, 0,0,0, 1,0,2, 6,1,0, 7,1)
phase = 0

BWESINE(start=0, dur, amp, freq, bw, phase, pan=0.5, wavet)
