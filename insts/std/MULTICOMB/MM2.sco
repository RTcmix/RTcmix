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

rtsetparams(44100, 2)
load("MULTICOMB")
rtinput("/snd/pablo1.snd")
reset(20000)
makegen(1, 24, 1000, 0,0, 0.5,1, 8.6,1, 8.7,0, 8.8,0)
MULTICOMB(0, 0, 8.8, 0.01, cpspch(7.02), cpspch(8.05), 5)
