// This shows how to "draw" randomly into the delay time table.  This
// is an experimental feature, so don't rely on it yet.  -JGG, 6/22/05

rtsetparams(44100, 2)
load("SPECTACLE2")

indur = 60
ringdur = 10

// ========================================================================
bus_config("MIX", "in 0", "aux 0 out")

// try all these...
//rtinput("../../snd/huhh.wav")
//rtinput("../../snd/conga.snd")
rtinput("../../snd/nucular.wav")

dur = DUR()
amp = 0.9
for (st = 0; st < indur; st += dur)
	MIX(st, 0, dur, amp, 0)
indur = st

// ========================================================================
wet = 1    // 100% wet

fftlen = 512            // yielding fftlen / 2 frequency bands
winlen = fftlen * 2     // the standard window length is twice FFT size
overlap = 2             // 2 hops per fftlen (4 per window)
window = 0              // use Hamming window

// no EQ
mineqfreq = 0
maxeqfreq = 0
eq = 0

// delay time -------------------------------------------------------------
mindelfreq = 0
maxdelfreq = 0

deltablen = 12    // changing this makes a big difference
deltimeL = maketable("literal", "nonorm", deltablen, 0)
deltimeR = copytable(deltimeL)

// left chan
randfreq = 9.5
seed = 1
index = makerandom("even", randfreq, min = 0, max = deltablen, seed)
value = makerandom("even", randfreq, min = 0.01, max = 5, seed)
value = makefilter(value, "smooth", lag = 30)
deltimeL = modtable(deltimeL, "draw", "literal", index, value, 0)

// right chan
randfreq = 9.0
seed += 1
index = makerandom("even", randfreq, min = 0, max = deltablen, seed)
value = makerandom("even", randfreq, min = 0.01, max = 5, seed)
value = makefilter(value, "smooth", lag = 30)
deltimeR = modtable(deltimeR, "draw", "literal", index, value, 0)
// ------------------------------------------------------------------------

// set feedback and overall gain using mouse
fb = makeconnection("mouse", "x", min=0, max=1, dflt=0, lag=50, "feedback")
amp = makeconnection("mouse", "y", min=-60, max=6, dflt=0, lag=50, "gain", "dB")
amp = makeconverter(amp, "ampdb")

bus_config("SPECTACLE2", "aux 0 in", "out 0")
SPECTACLE2(start=0, inskip=0, indur, amp, iamp=1, ringdur, fftlen, winlen,
	window, overlap, eq, deltimeL, fb, mineqfreq, maxeqfreq, mindelfreq,
	maxdelfreq, 0, wet, inchan=0, pan=1)

bus_config("SPECTACLE2", "aux 0 in", "out 1")
SPECTACLE2(start=0, inskip=0, indur, amp, iamp=1, ringdur, fftlen, winlen,
	window, overlap, eq, deltimeR, fb, mineqfreq, maxeqfreq, mindelfreq,
	maxdelfreq, 0, wet, inchan=0, pan=0)

