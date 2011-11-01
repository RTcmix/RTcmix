// This is an example of walking slowly through both input and impulse
// response files, convolving a little bit at a time.    -JG

writeit = 0
if (writeit)
	set_option("play = off", "clobber = on")
rtsetparams(44100, 2)
load("CONVOLVE1")
load("PANECHO")
bus_config("CONVOLVE1", "in 0", "aux 0 out")
bus_config("PANECHO", "aux 0 in", "out 0-1")

snddir = "../../snd/"
outdir = "./"

sf1 = snddir + "huhh.wav"
sf2 = snddir + "loocher.aiff"
sf3 = snddir + "nucular.wav"

if (writeit)
	rtoutput(outdir + "CONVOLVE1_2.wav")
rtinput(sf1)
imp_tab = maketable("soundfile", "nonorm", 0, sf3)
//plottable(imp_tab)

outdur = 14

// out_increment affects the "grain" of the output file. It's the skip
// between successive convolve notes. A smaller number will be more
// smeary and take longer.
out_increment = 0.02

// src_dur is the amount of the source file to process at a time.
// src_inskip is where to start reading.
// src_increment is how much to increase inskip between convolve notes.
// src_env is the envelope applied to the segment of the source file.
src_amp = 2.0
src_dur = 0.15
src_inskip = 0
src_increment = src_dur * 0.02
src_maxjitter = 0.0
src_env = maketable("window", 1000, "hanning")

// impulse response file (same as for source file)
imp_amp = 2.0
imp_dur = 0.1
imp_inskip = 0
imp_increment = imp_dur * 0.01
imp_maxjitter = 0.0

// window function used for both input and impulse response
window = maketable("window", 1000, "hanning")

// general parameters
wet = 1.0
inchan = 0
control_rate(20000)

inskip = src_inskip
impskip = imp_inskip
for (start = 0; start < outdur; start += out_increment) {
	printf("pointers: src=%f, imp=%f, out=%f\n", inskip, impskip, start)
	CONVOLVE1(start, inskip, src_dur, src_amp * src_env,
					imp_tab, impskip, imp_dur, imp_amp, window,
					wet, inchan, pan=1)
	inskip += src_increment + irand(0, src_maxjitter)
	impskip += imp_increment + irand(0, imp_maxjitter)
}

env = maketable("line", 1000, 0,0, 4,1, outdur-1,1, outdur,0)
PANECHO(0, 0, outdur, env, ldel=0.2, rdel=0.77, fb=0.2, ringdur=4.0)

