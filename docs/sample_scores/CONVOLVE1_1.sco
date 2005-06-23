rtsetparams(44100, 2)
load("CONVOLVE1")

rtinput("../../snd/nucular.wav")
inchan = 0
inskip = 0.0
indur = DUR()

// random impulse response
impdur = 0.3
imptab = maketable("random", "nonorm", impdur * 44100, "even",
				min = -10000, max = 10000, seed = 1)
impskip = 0.0
impgain = 4.5

amp = maketable("line", 2000, 0,0, .1,1, indur-.1,1, indur,0)
wintab = 0
wetpct = 1.0

CONVOLVE1(0, inskip, indur, amp, imptab, impskip, impdur, impgain,
		wintab, wetpct, inchan, pan=1)

// decorrelate channels by shifting noise samples by half table length
// could also create a new table with a different seed
imptab = copytable(modtable(imptab, "shift", tablelen(imptab) / 2))

CONVOLVE1(0, inskip, indur, amp, imptab, impskip, impdur, impgain,
		wintab, wetpct, inchan, pan=0)
