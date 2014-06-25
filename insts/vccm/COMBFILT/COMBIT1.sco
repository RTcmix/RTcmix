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

load("COMBIT")
rtsetparams(44100, 2)
rtinput("/snd/Public_Sounds/jimi3dSoundDN.aiff")
dur = DUR()
COMBIT(0, 0, dur, 1, cpspch(7.09), .5, 0, 0.5)

