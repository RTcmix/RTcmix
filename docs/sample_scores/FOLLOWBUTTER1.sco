rtsetparams(44100, 2)
load("WAVETABLE")
load("FOLLOWBUTTER")

source_listen = 0  // set to 1 to hear carrier and modulator separately

dur = 20

// play carrier to bus 0
bus_config("WAVETABLE", "aux 0 out")
wavet = maketable("wave", 8000, "buzz10")
amp = 15000
WAVETABLE(0, dur, amp, freq = 140, 0, wavet)
WAVETABLE(0, dur, amp, freq * 1.002, 0, wavet)

// play modulator to bus 1
bus_config("WAVETABLE", "aux 1 out")
env = maketable("line", 1000, 0,0, 1,1, 2,0)
reset(20000)
srand(2)
incr = base_incr = 0.14
notedur = base_incr * 0.35
freq = 1000
for (st = 0; st < dur; st += incr) {
   db = irand(40, 92)
   WAVETABLE(st, notedur, ampdb(db) * env, freq, 0, wavet)
   incr = base_incr * irand(0.25, 4)
}
reset(1000)

// apply modulator's amp envelope to carrier
bus_config("FOLLOWBUTTER", "aux 0-1 in", "out 0-1")
env = maketable("line", 1000, 0,0, 1,1, 19,1, 20,0)
caramp = 4.0
modamp = 1.5
winlen = 10         // number of samples for power gauge to average
smooth = 0.8        // how much to smooth the power gauge curve
type = "bandpass"   // "lowpass", "highpass", "bandpass", "bandreject"
mincf = 120
maxcf = 12000
bw = -0.3
steepness = 2
pan = 0.5
if (source_listen) {
   bus_config("MIX", "aux 0-1 in", "out 0-1")
   MIX(0, 0, dur, 1, 0, 1)
}
else
   FOLLOWBUTTER(0, inskip = 0, dur, caramp, modamp, winlen, smooth, type,
      mincf, maxcf, steepness, pan, bw)

