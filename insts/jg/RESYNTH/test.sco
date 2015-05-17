outchans = 2
writeit = 0
if (writeit) {
	set_option("clobber = on", "play = off")
	rtsetparams(sr=44100, outchans)
	rtoutput("test.wav", "24")
}
else
	rtsetparams(sr=44100, outchans)
load("RESYNTH")
if (outchans > 1)
	bus_config("RESYNTH", "out 0-" + (outchans - 1))
control_rate(1000)

timescale = 4.0
//amp = maketable("line", "nonorm", 1000, 0,0, 1,50000, 2,50000)
amp = 50000
SDIFfilename = "nucular.sdif"
dur = 1.578957 * timescale
SDIFinskip = 0	// unsupported
time = maketable("curve", "nonorm", 1000, 0,0,0, 1,dur/timescale)
//time = maketable("curve", "nonorm", 1000, 0,dur/timescale,0, 1,0)
//time = maketable("random", "nonorm", 20, "linear", min=0.1, max=1.5, seed=1)
//time = makeLFO("sine", freq=0.5, min=0.1, max=1.5)
//time = makeconnection("display", time, "time")
interpTime = 0.02 * timescale
//minfreq = maketable("line", "nonorm", 1000, 0,50, 1,50, 3,1000)
//minfreq = 1100
minfreq = 50
maxfreq = sr / 2
ampthresh = -65
freqscale = 1.0
//freqscale = maketable("line", "nonorm", 1000, 0,1, 1,1, 3,2, 4,1, 6,.7)
freqoffset = 0
//freqoffset = maketable("line", "nonorm", 1000, 0,-40.0, 1,-50, 3,0, 4,0, 6,100)
//freqoffset = -1000
/*
retuneTable = maketable("literal", "nonorm", 0,
   6.00, 6.07, 7.00, 7.07, 8.00, 8.07, 9.00, 9.07,
   10.00, 10.07, 11.00, 11.07, 12.00)
*/
retuneTable = maketable("literal", "nonorm", 0,	// in octave.pc
	7.00, 7.02, 7.04, 7.05, 7.07, 7.09, 7.11,
	8.00, 8.02, 8.04, 8.05, 8.07, 8.09, 8.11,
	9.00, 9.02, 9.04, 9.05, 9.07, 9.09, 9.11,
	10.00, 10.02, 10.04, 10.05, 10.07, 10.09, 10.11)
//retuneTable = 0
retuneTranspose = 0.0
retuneSensitivity = 3.0
retuneStrength = 0.9
//retuneStrength = maketable("line", 1000, 0,0, 1,1, 2,1)
panseed = 1
RESYNTH(0, dur, amp, SDIFfilename, SDIFinskip, time, interpTime, minfreq,
	maxfreq, ampthresh, freqscale, freqoffset, retuneTable, retuneTranspose,
	retuneSensitivity, retuneStrength, panseed)

