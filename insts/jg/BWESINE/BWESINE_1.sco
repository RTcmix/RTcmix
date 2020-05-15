rtsetparams(44100, 2)
load("./libBWESINE.so")

maxamp = 8000
amp = maketable("line", "nonorm", 1000, 0,0, 1,maxamp, 9,maxamp, 10,0)
wavet = maketable("wave", 32767, "sine")
bw = maketable("line", "nonorm", 1000, 0,0, 1,0, 6,1, 7,1)
phase = 0

BWESINE(start=0, dur=5, amp, freq=440, bw, phase, pan=0.5, wavet)
