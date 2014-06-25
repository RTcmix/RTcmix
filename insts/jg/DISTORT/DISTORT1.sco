rtsetparams(44100, 2)
load("WAVETABLE")
load("DISTORT")

bus_config("WAVETABLE", "aux 0 out")
bus_config("DISTORT", "aux 0 in", "out 0-1")

dur = 6.0
amp = 10000
pitch = 7.00
wavet = maketable("wave", 15000, "saw20")
control_rate(10000)
WAVETABLE(0, dur, amp, pitch, pan=0, wavet)
WAVETABLE(0.1, dur, amp, pitch+.0002, pan, wavet)

// distort wavetable output
bypass = 0
type = 1
amp = 0.2
gain = 20.0
cf = 0
env = maketable("line", 1000, 0,0, 1,1, 7,1, 10,0)
DISTORT(0, 0, dur, amp * env, type, gain, cf, 0, pan=.5, bypass)

