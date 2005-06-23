rtsetparams(44100, 2)
load("FREEVERB")

bus_config("MIX", "in 0", "aux 0 out")
bus_config("FREEVERB", "aux 0 in", "out 0-1")

//-----------------------------------------------------------------
rtinput("../../snd/nucular.wav")
inskip = 0
dur = DUR()

for (start = 0; start < 12; start += dur)
   MIX(start, inskip, dur, amp=1, 0)

//-----------------------------------------------------------------
outskip = 0
inskip = 0     // must be zero, since it's reading from aux bus
dur += start
amp = 1.0
env = maketable("line", 1000, 0,.1, 4,1, 10,1)

roomsize = maketable("curve", "nonorm", 100, 0,0,-2, 6,1.1,0, 8,0)
predelay = .0
ringdur = 9
damp = maketable("line", "nonorm", 100, 0,100, 1,100, 4,20, 5,0)
dry = 40
wet = 32
width = maketable("line", "nonorm", 100, 0,0, 3,100, 6,100)

FREEVERB(outskip, inskip, dur, amp * env, roomsize, predelay, ringdur,
         damp, dry, wet, width)

