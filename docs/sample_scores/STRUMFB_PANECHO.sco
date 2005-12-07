// Control guitar feedback instrument with mouse.  Takes a while to learn
// how to do this.  Experiment.  If things are getting out of control, park
// mouse somewhere at bottom of window.  For more feedback, park mouse
// elsewhere.  Sometimes takes a while for feedback to build up.  -JGG

rtsetparams(44100, 2, 256)
load("STRUMFB")
load("PANECHO")

dur = 120
use_frets = 0

freq = makeconnection("mouse", "x", min=5, max=8, min, lag=80, "freq")
if (use_frets) {
	freq = makefilter(freq, "quantize", 1 / 12)
	freq = makefilter(freq, "smooth", lag=40)
}
freq = makeconverter(freq, "pchoct")

fbfreq = makeconnection("mouse", "y", min=5, max=9, min, lag=80, "fbfreq")
fbfreq = makeconverter(fbfreq, "pchoct")

fbgain = makeconnection("mouse", "y", min=0, max=1, min, lag=80, "fbgain")

decaytime = 2.0
nyqdecaytime = 1
distgain = 10
cleanlevel = 0
distlevel = 1
// envelope mutes initial blast caused by freq lag filters
amp = 10000 * maketable("line", 1000, 0,0, 2,0, 3,1, dur-2,1, dur,0)
squish = 1

bus_config("STRUMFB", "aux 0 out")
STRUMFB(0, dur, amp, freq, fbfreq, squish, decaytime, nyqdecaytime,
	distgain, fbgain, cleanlevel, distlevel)

bus_config("PANECHO", "aux 0 in", "out 0-1")
PANECHO(0, 0, dur, 1, ld=0.05, rd=0.12, fb=.2, rdur=2)

