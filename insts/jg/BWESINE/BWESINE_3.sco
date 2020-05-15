// for testing phase argument
set_option("clobber=on", "play=off")
rtsetparams(44100, 1)
rtoutput("sine-phase-90.wav")
load("./libBWESINE.so")

amp = 10000
wavet = maketable("wave", 1000, "sine")
bw = 0

pi = 3.141592653589793
phase = -pi/2

BWESINE(start=0, dur=5, amp, freq=10, bw, phase, pan=0, wavet)
