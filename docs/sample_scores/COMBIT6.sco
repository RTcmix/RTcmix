rtsetparams(44100, 2)
load("COMBIT")

totdur = 15
masteramp = 1.0

//-----------------------------------------------------------------------------
bus_config("MIX", "in 0", "aux 0 out")
rtinput("../../snd/huhh.wav")
inskip = 0
dur = DUR()
amp = maketable("curve", 1000, 0,0,1, 1,1,0, 3,1,-1, 4,0) * 0.5

increment = base_increment = dur * 0.5
for (start = 0; start < totdur; start += increment) {
   MIX(start, inskip, dur, amp, 0)
   increment = base_increment + irand(-.5, .5)
}

//-----------------------------------------------------------------------------
bus_config("COMBIT", "aux 0 in", "out 0-1")
totdur += dur
amp = masteramp
env = maketable("line", 1000, 0,0, 1,1, 7,1, 10,0)

freq = maketable("random", "nonorm", totdur * 8, "cauchy", 50, 180, 1)
rvbtime = maketable("line", "nonorm", 1000, 0,2, 2,10, 3,5)
pan = maketable("wave", "nonorm", 1000, .5) + 0.5	// vals btw. 0 and 1
ringdur = 0.5
control_rate(2000)

COMBIT(0, 0, totdur, amp * env, freq, rvbtime, 0, pan, ringdur)


// -JGG, 7/12/04

