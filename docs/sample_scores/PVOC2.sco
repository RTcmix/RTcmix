rtsetparams(44100, 1)
load("PVOC")

rtinput("../../snd/nucular.wav")

start = 0
inskip = 0
amp = 1
inchan = 0
fftlen = 1024
window = fftlen * 2
interpolation = 100	// can be no larger than <window>
filedur = DUR() - 0.5;	// file has silence at end

// start at regular rate, and slow to 1/50th of that by halfway
decimation = maketable("line", "nonorm", 8192, 0, interpolation, 1, interpolation/50, 2, interpolation/50);

// with fixed values, dur will be indur * interpolation / decimation.  When
// using a table, we have to use the area under the curve to determine length.

dur = filedur * (1 + 50 + 50) / 3;
npoles = 0

bus_config("PVOC", "in 0", "out 0")
PVOC(start, inskip, dur, amp, inchan, fftlen, window, decimation, interpolation, 0, npoles)

