rtsetparams(44100, 2)
load("EQ")

rtinput("mystereofile.wav")
inskip = 0
dur = DUR()
amp = 1
bypass = 0
type = "lowshelf"
freq = 100
Q = 3.0
gain = 6.0

EQ(0, inskip, dur, amp, type, 0, 1, bypass, freq, Q, gain)
EQ(0, inskip, dur, amp, type, 1, 0, bypass, freq, Q, gain)

