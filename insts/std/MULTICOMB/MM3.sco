/* multicomb -- 4 simultaneous comb filters randomly chosen within
*	a specified range (spread across a stereo field)
*  p0 = output skip
*  p1 = input skip
*  p2 = output duration
*  p3 = amplitude multiplier
*  p4 = comb frequency range bottom
*  p5 = comb frequency range top
*  p6 = reverb time
*  assumes function table 1 is the amplitude envelope
*
*/

set_option("full_duplex_on")
rtsetparams(44100, 2, 256)
load("MULTICOMB")
rtinput("AUDIO")
reset(20000)

srand(87)

makegen(1, 24, 1000, 0,0, 1,1, 3,1, 5,0)
for(start = 0; start < 30; start = start + 2.5) {
	low = random() * 500.0 + 50.0
	hi = low + (random() * 300.0)
	MULTICOMB(start, 0, 5, 0.01, low, hi, 2.5)
	}
