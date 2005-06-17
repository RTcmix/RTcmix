/* p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; 
   p4-n = channel mix matrix
   we're stashing the setline info in gen table 1 */

rtsetparams(44100, 2)
rtinput("../../../snd/input.wav")
filedur = DUR()
setline(0,0, .1, 1, 2,0)

reset(10000)
dur = 1
outsk = 0.0
for(i = 0; i < 550; i = i + 1) {
	insk = random() * filedur
	dur = random() * 0.2

	if (random() > 0.5) ch1 = 0
	else ch1 = 1
	if (random() > 0.5) ch2 = 0
	else ch2 = 1
	MIX(outsk, insk, dur, 1, ch1, ch2)
	outsk = outsk + 0.05
}

