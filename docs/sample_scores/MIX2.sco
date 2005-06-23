/* p0 = outsk; p1 = insk; p2 = duration (-endtime); p3 = amp; 
   p4-n = channel mix matrix
   we're stashing the setline info in gen table 1 */

rtsetparams(44100, 2)
rtinput("../../snd/input.wav")
filedur = DUR()

amp = 1.0
env = maketable("line", 1000, 0,0, .2,1, 2,0)

reset(10000)
dur = 1;
for (outsk = 0; outsk < 14.0; outsk += dur) {
	insk = random() * filedur
	dur = random() * 0.2

	if (random() > 0.5)
		ch1 = 0
	else
		ch1 = 1
	if (random() > 0.5)
		ch2 = 0
	else
		ch2 = 1

	MIX(outsk, insk, dur, amp * env, ch1, ch2)
}

