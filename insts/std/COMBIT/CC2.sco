/* combit --  comb filter instrument
*
* p0 = output skip
* p1 = input skip
* p2 = input duration
* p3 = amplitude multiplier
* p4 = pitch (cps)
* p5 = reverb time
* p6 = input channel [optional]
* p7 = stereo spread [optional]
*
*/

rtsetparams(44100, 2)
load("COMBIT")
rtinput("../../../snd/input.wav");

filedur = DUR(0);
dur = 0.1
setline(0,0, 0.1,1, 1,0) 
reset(1000)
for (outsk = 0; outsk < 14.0; outsk = outsk + 0.1) {
	insk = random() * filedur
	pitch = random() * 500 + 100
	COMBIT(outsk, insk, dur, 0.1, pitch, .5, 0, random());
	}
