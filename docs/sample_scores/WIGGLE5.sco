// See "WIGGLE1.sco" for a description of parameters

rtsetparams(44100, 2)
load("WIGGLE")

dur = 40.0
gaindb = 85.0
pitch = 9.01
mod_freq = cpspch(pitch + 10.00)

env = maketable("curve", 2000, 0,0,1, dur*.08,1,0, dur*.7,1,-3, dur,0)
amp = ampdb(gaindb)

carwav = maketable("wave", 5000, 1, .4, .3, .2, .1, .1, .05, .01)
modwav = maketable("wave", 2000, "sine")
mod_depth = maketable("line", "nonorm", 1000, 0,0, 1,7)
pan = maketable("line", "nonorm", 1000, 0,.1, 1,.5)

depth_type = 2
filt_type = 1
filt_steep = 12
balance = false
filt_cf = maketable("line", "nonorm", 1000, 0,3000, 1,8000, 3,900)

WIGGLE(start=0, dur, amp * env, pitch, depth_type, filt_type, filt_steep,
	balance, carwav, modwav, mod_freq, mod_depth, filt_cf, pan)

gliss = maketable("line", "nonorm", 1000, 0,0, 1,0.001)
freq = makeconverter(octpch(pitch) + gliss, "cpsoct")
pan = maketable("line", "nonorm", 1000, 0,.6, 1,1)
WIGGLE(start+.01, dur, amp * env, freq, depth_type, filt_type, filt_steep,
	balance, carwav, modwav, mod_freq, mod_depth, filt_cf, pan)

// bass later on
start = start + (dur * .4)
dur = (dur - start) + 2
env = maketable("curve", 2000, 0,0,1, dur*.5,1,0, dur*.7,1,-3, dur,0)
gaindb = 75.0
pitch = 5.10
carwav = maketable("wave", 2000, 1, .5, .3, .1)
amp = ampdb(gaindb)
WIGGLE(start, dur, amp * env, pitch, depth_type=0, filt_type=0, filt_steep,
	balance, carwav, modwav, mod_freq, mod_depth, filt_cf, pan=0)
pitch += 0.001
WIGGLE(start, dur, amp * env, pitch, depth_type=0, filt_type=0, filt_steep,
	balance, carwav, modwav, mod_freq, mod_depth, filt_cf, pan=1)
