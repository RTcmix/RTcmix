rtsetparams(44100, 2)
load("WAVETABLE")
load("FOLLOWBUTTER")

source_listen = 0  /* set to 1 to hear carrier and modulator separately */

dur = 20

/* play carrier to bus 0 */
bus_config("WAVETABLE", "aux 0 out")
makegen(2, 10, 8000, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1)
amp = 15000
WAVETABLE(0, dur, amp, freq = 140)
WAVETABLE(0, dur, amp, freq * 1.002)

/* play modulator to bus 1 */
bus_config("WAVETABLE", "aux 1 out")
setline(0,0, 1,1, 2,0)
reset(20000)
srand(2)
incr = base_incr = 0.14
notedur = base_incr * 0.35
freq = 1000
for (st = 0; st < dur; st = st + incr) {
   db = irand(40, 92)
   WAVETABLE(st, notedur, ampdb(db), freq)
   incr = base_incr * irand(0.25, 4)
}
reset(1000)

/* apply modulator's amp envelope to carrier */
bus_config("FOLLOWBUTTER", "aux 0-1 in", "out 0-1")
setline(0,0, 1,1, 19,1, 20,0)
caramp = 4.0
modamp = 1.5
winlen = 10       /* number of samples for power gauge to average */
smooth = 0.8      /* how much to smooth the power gauge curve */
type = 3          /* 1: lowpass, 2: highpass, 3: bandpass, 4: bandreject */
mincf = 120
maxcf = 12000
bw = -.3
steepness = 2
pctleft = 0.5
makegen(2, 18, 10, 0, bw, 1, bw)
if (source_listen) {
   bus_config("MIX", "aux 0-1 in", "out 0-1")
   MIX(0, 0, dur, 1, 0, 1)
}
else
   FOLLOWBUTTER(0, inskip = 0, dur, caramp, modamp, winlen, smooth, type,
      mincf, maxcf, steepness, pctleft)

