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

rtsetparams(44100, 2, 512)
load("COMBIT")
rtinput("../../../snd/huhh.wav")
pitch = maketable("line", "nonorm", 50000, 0, 600, 1, 60);
setline(0,0, 2,1, 3.5,1)
COMBIT(0, 0, 3.5, 0.8, pitch, 0.1, 0, 0.5)
