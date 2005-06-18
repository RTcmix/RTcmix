// See "WIGGLE1.sco" for a description of parameters

rtsetparams(44100, 2)
load("WIGGLE")

dur = 25
amp = 3000
pitch = 10.00

env = maketable("curve", 2000, 0,0,2, dur*.1,1,0, dur*.4,1,-3, dur,0)

car_wavetable = maketable("wave", 8000, "sine")

min = -1.00
max = 2.00
seed = 1
gliss = maketable("random", "nonorm", "nointerp", 300, "high", min, max)
freq = makeconverter(octpch(pitch) + gliss, "cpsoct")

mod_depth_type = 2
mod_wavetable = maketable("wave", 1000, "sine")
mod_freq = 200
mod_depth = 20

filt_cf = maketable("curve", "nonorm", 2000, 0,1000,-4, 1,1)
pan = maketable("line", "nonorm", 1000, 0,.2, 1,.5, 2,1)

filt_type = 2      // highpass
filt_steep = 20
balance = false

WIGGLE(st=0, dur, amp * env, freq, mod_depth_type, filt_type, filt_steep,
	balance, car_wavetable, mod_wavetable, mod_freq, mod_depth, filt_cf, pan)

pan = maketable("line", "nonorm", 1000, 0,1, 1,0)
freq = makeconverter(octpch(pitch + 0.01) + gliss, "cpsoct")
WIGGLE(st=0.1, dur, amp * env, freq, mod_depth_type, filt_type, filt_steep,
	balance, car_wavetable, mod_wavetable, mod_freq, mod_depth, filt_cf, pan)
