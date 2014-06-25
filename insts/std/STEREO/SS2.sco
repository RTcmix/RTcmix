/* p0 = outsk; p1 = insk; p2 = dur (normal) OR if - , endtime; p3 = amp; 
   p4-n = channel mix matrix
   we're stashing the setline info in gen table 1 */

rtsetparams(44100, 2)
load("STEREO")
rtinput("/snd/pablo1.snd")
setline(0,0, .1, 1, 2,0)

reset(10000)
dur = 1;
for(outsk = 0; outsk < 10.0; outsk = outsk + dur) {
	insk = random() * 7.0
	dur = random() * 0.2
	STEREO(insk, outsk, dur, 1, random(), -1)
}

