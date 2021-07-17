rtsetparams(44100, 2)
load("WAVETABLE")

maxamp = 8000
amp = maketable("line", "nonorm", 1000, 0,0, 1,maxamp, 9,maxamp, 10,0)
wavet = maketable("wave", 32767, "sine")

WAVETABLE(start=0, dur=5, amp, freq=440, pan=0.5, wavet)
