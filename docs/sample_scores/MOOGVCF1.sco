rtsetparams(44100, 2)
load("WAVETABLE")
load("MOOGVCF")

// feed wavetable into filter
bus_config("WAVETABLE", "aux 0 out")
bus_config("MOOGVCF", "aux 0 in", "out 0-1")

dur = 10.0
amp = 10000
pitch = 6.00
wavet = maketable("wave", 15000, "saw30")
WAVETABLE(0, dur, amp, pitch, 0, wavet)
WAVETABLE(0, dur, amp, pitch+.0005, 0, wavet)

amp = 3.0
env = maketable("line", 1000, 0,1, 7,1, 10,0)

lowcf = 500
highcf = 1500
lowres = 0.1
highres = 0.9

cf = maketable("line", "nonorm", 2000,
			0,lowcf, dur*.2,lowcf, dur*.5,highcf, dur,lowcf)
res = maketable("line", "nonorm", 2000, 0,lowres, 1,highres, 2,lowres)

MOOGVCF(0, 0, dur, amp * env, inchan=0, pan=0.5, bypass=0, cf, res)

