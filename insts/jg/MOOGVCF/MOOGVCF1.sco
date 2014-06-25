rtsetparams(44100, 1)
load("WAVETABLE")
load("MOOGVCF")

/* feed wavetable into filter */
bus_config("WAVETABLE", "aux 0 out")
bus_config("MOOGVCF", "aux 0 in", "out 0")

dur = 10.0
amp = 10000
pitch = 6.00
makegen(2, 10, 15000,
   1, 1/2, 1/3, 1/4, 1/5, 1/6, 1/7, 1/8, 1/9, 1/10, 1/11, 1/12,
   1/13, 1/14, 1/15, 1/16, 1/18, 1/19, 1/20, 1/21, 1/22, 1/23, 1/24)  /* saw */
reset(10000)
WAVETABLE(0, dur, amp, pitch)
WAVETABLE(0, dur, amp, pitch+.0005)

amp = 2.0
lowcf = 500
highcf = 1200
lowres = 0.1
highres = 0.9

setline(0,1, 7,1, 10,0)
makegen(2, 18, 2000, 0,lowcf, dur*.2,lowcf, dur*.5,highcf, dur,lowcf)
makegen(3, 18, 2000, 0,lowres, 1,highres, 2,lowres)

MOOGVCF(0, 0, dur, amp)

