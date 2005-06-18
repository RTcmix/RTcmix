// See "sample_scores/single_insts/WIGGLE1.sco" for a description of parameters

rtsetparams(44100, 2)
load("WIGGLE")
load("FREEVERB")
bus_config("WIGGLE", "aux 0-1 out")
bus_config("FREEVERB", "aux 0-1 in", "out 0-1")

dur = 18

// --------------------------------------------------------------- wiggle ---
amp = 7000
env = maketable("line", 1000, 0,0, 1,1, 6,1, 8,0)
pitch = 7.00

car_wavetable = maketable("wave", 8000, "buzz19")
mod_wavetable = maketable("wave", 80, 1, 1, .4)
mfreq = cpspch(pitch + 1)
mod_freq = maketable("line", "nonorm", 20, 0,mfreq, 1,mfreq, 2,mfreq-18)
mod_depth = maketable("line", "nonorm", 2000, 0,1.5, 1,1.8)
filt_cf = maketable("line", "nonorm", 2000, 0,4000, 1,10000, 2,100)
pan = maketable("line", "nonorm", 1000, 0,0, 1,.5)

depth_type = 2  // mod index
filt_type = 1
filt_steep = 5
balance = false

WIGGLE(st = 0, dur, amp * env, pitch, depth_type, filt_type, filt_steep,
	balance, car_wavetable, mod_wavetable, mod_freq, mod_depth, filt_cf, pan)

min = -0.04
max = 0.04
type = "gaussian"
gliss = maketable("random", "nonorm", "nointerp", 250, type, min, max, seed=1)
freq = makeconverter(octpch(pitch) + gliss, "cpsoct")
pan = maketable("line", "nonorm", 2000, 0,1, 1,0)
WIGGLE(st = 0.01, dur, amp * env, freq, depth_type, filt_type, filt_steep,
	balance, car_wavetable, mod_wavetable, mod_freq, mod_depth, filt_cf, pan)


// --------------------------------------------------------------- reverb ---
roomsize = 0.85
predelay = 0.02
ringdur = 1.0
damp = 50
dry = 80
wet = 30
stwid = 100

FREEVERB(0, 0, dur, amp=1, roomsize, predelay, ringdur, damp, dry, wet, stwid)
