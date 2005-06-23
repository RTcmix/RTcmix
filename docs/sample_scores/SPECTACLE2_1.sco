rtsetparams(44100, 2)
load("SPECTACLE2")

rtinput("../../snd/huhh.wav")
inchan = 0
inskip = 0
indur = DUR()
ringdur = 15           // play after indur elapses, while delay lines flush
amp = 3.5
wet = 1                // 100% wet

fftlen = 1024          // yielding 512 frequency bands
winlen = fftlen * 2    // the standard window length is twice FFT size
overlap = 2            // 2 hops per fftlen (4 per window)
window = 0             // use Hamming window

// input envelope (spanning <indur>)
ienv = maketable("line", 1000, 0,0, 1,1, 19,1, 20,0)

// output envelope (spanning <indur> + <ringdur>)
oenv = maketable("curve", 1000, 0,1,0, 2,1,-1, 3,0)

eqtablen = fftlen / 2
mineqfreq = 0
maxeqfreq = 0

// EQ curve: -90 dB at 0 Hz, ramping up to 0 dB at 200 Hz, etc.
eq = maketable("line", "nonorm", eqtablen, 0,-90, 200,0, 8000,-3, 22050,-6)

deltablen = fftlen / 2
mindelfreq = 0
maxdelfreq = 0

// fixed delay times between .4 and 3, randomly spread across spectrum
min = .4
max = 3
seed = 1
deltime = maketable("random", "nonorm", deltablen, "even", min, max, seed)

// constant feedback of 90% for all freq. bands
fb = .9

// do it for the left chan
SPECTACLE2(0, inskip, indur, amp * oenv, ienv, ringdur, fftlen, winlen,
	window, overlap, eq, deltime, fb, mineqfreq, maxeqfreq,
	mindelfreq, maxdelfreq, 0, wet, inchan, pan=1)

// shift delay table to decorrelate channels
deltime = copytable(modtable(deltime, "shift", 2))

// do it for the right chan
SPECTACLE2(0, inskip, indur, amp * oenv, ienv, ringdur, fftlen, winlen,
	window, overlap, eq, deltime, fb, mineqfreq, maxeqfreq,
	mindelfreq, maxdelfreq, 0, wet, inchan, pan=0)


