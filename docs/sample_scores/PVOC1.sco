rtsetparams(44100, 2)
load("PVOC")

rtinput("../../snd/nucular.wav")

transposition_left = -0.01	// oct.pc
transposition_right = 0.01
timescale = 4.0

start = 0
inskip = 0
dur = DUR() * timescale
amp = 2
inchan = 0
fftlen = 1024
window = fftlen * 2
interpolation = window	// can be no larger than <window>
decimation = interpolation / timescale
npoles = 0

bus_config("PVOC", "in 0", "out 0")
pitch_multiplier = cpspch(transposition_left) / cpspch(0.0)
PVOC(start, inskip, dur, amp, inchan, fftlen, window, decimation,
	interpolation, pitch_multiplier, npoles)

bus_config("PVOC", "in 0", "out 1")
pitch_multiplier = cpspch(transposition_right) / cpspch(0.0)
PVOC(start, inskip, dur, amp, inchan, fftlen, window, decimation,
	interpolation, pitch_multiplier, npoles)

